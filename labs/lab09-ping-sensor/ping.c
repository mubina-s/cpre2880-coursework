/**
 * Driver for ping sensor
 * @file ping.c
 */

#include "ping.h"
#include "Timer.h"
#include "driverlib/interrupt.h"

#define PING_PORTB_MASK         0x02      // Port B clock bit
#define PING_PIN_MASK           0x08      // PB3
#define TIMER3_MASK             0x08      // Timer 3 clock bit

#define TIMER3B_INT_MASK        0x400     // Capture B event interrupt
#define TIMER3B_ENABLE_MASK     0x100     // TBEN

#define TIMER3B_EVENT_MASK      0x0C00
#define TIMER3B_EVENT_RISING    0x0000
#define TIMER3B_EVENT_FALLING   0x0400

#define TIMER3B_24BIT_MAX       0x01000000u

// Global shared variables
volatile uint32_t g_start_time = 0;
volatile uint32_t g_end_time = 0;
volatile uint32_t g_last_pulse_ticks = 0;
volatile uint32_t g_overflow_count = 0;
volatile bool g_last_overflow = false;
volatile float g_last_pulse_ms = 0.0f;

static volatile enum { LOW, HIGH, DONE } g_state = LOW;

// Configure PB3 as normal GPIO output for trigger pulse
static void ping_config_pb3_gpio_output(void)
{
    GPIO_PORTB_AFSEL_R &= ~PING_PIN_MASK;                 // disable alternate function
    GPIO_PORTB_PCTL_R  &= ~0x0000F000;                    // clear PMC3
    GPIO_PORTB_AMSEL_R &= ~PING_PIN_MASK;                 // disable analog
    GPIO_PORTB_DIR_R   |= PING_PIN_MASK;                  // output
    GPIO_PORTB_DEN_R   |= PING_PIN_MASK;                  // digital enable
}

// Configure PB3 as Timer3B capture input
static void ping_config_pb3_timer_capture(void)
{
    GPIO_PORTB_AMSEL_R &= ~PING_PIN_MASK;                 // disable analog
    GPIO_PORTB_DEN_R   |= PING_PIN_MASK;                  // digital enable
    GPIO_PORTB_DIR_R   &= ~PING_PIN_MASK;                 // input
    GPIO_PORTB_AFSEL_R |= PING_PIN_MASK;                  // alternate function
    GPIO_PORTB_PCTL_R   = (GPIO_PORTB_PCTL_R & ~0x0000F000) | 0x00007000; // PB3 = T3CCP1
}

void ping_init(void)
{
    timer_init();

    // Enable clocks
    SYSCTL_RCGCGPIO_R  |= PING_PORTB_MASK;
    SYSCTL_RCGCTIMER_R |= TIMER3_MASK;

    while ((SYSCTL_PRGPIO_R & PING_PORTB_MASK) == 0) {}
    while ((SYSCTL_PRTIMER_R & TIMER3_MASK) == 0) {}

    // Start with PB3 as GPIO output, low
    ping_config_pb3_gpio_output();
    GPIO_PORTB_DATA_R &= ~PING_PIN_MASK;

    // Disable Timer3B during setup
    TIMER3_CTL_R &= ~TIMER3B_ENABLE_MASK;

    // Timer configured as 16-bit timer
    TIMER3_CFG_R = 0x4;

    // Timer B: capture mode, edge-time mode, count-down
    TIMER3_TBMR_R = 0x7;          // capture mode + edge-time + count-down
    TIMER3_TBMR_R &= ~0x10;       // make sure count direction is down

    // Start by listening for rising edge
    TIMER3_CTL_R = (TIMER3_CTL_R & ~TIMER3B_EVENT_MASK) | TIMER3B_EVENT_RISING;

    // 24-bit timer using prescaler + interval load
    TIMER3_TBILR_R = 0xFFFF;
    TIMER3_TBPR_R  = 0xFF;

    // Clear interrupt and keep masked until trigger
    TIMER3_ICR_R   = TIMER3B_INT_MASK;
    TIMER3_IMR_R  &= ~TIMER3B_INT_MASK;

    // Register ISR and enable NVIC
    IntRegister(INT_TIMER3B, TIMER3B_Handler);

    NVIC_PRI9_R = (NVIC_PRI9_R & ~0x000000E0) | 0x00000020;   // priority 1
    NVIC_EN1_R |= 0x10;                                       // enable IRQ 36 = Timer3B

    IntMasterEnable();
}

void ping_trigger(void)
{
    g_state = LOW;
    g_start_time = 0;
    g_end_time = 0;
    g_last_pulse_ticks = 0;
    g_last_overflow = false;

    // Disable timer and interrupt while sending trigger pulse
    TIMER3_CTL_R &= ~TIMER3B_ENABLE_MASK;
    TIMER3_IMR_R &= ~TIMER3B_INT_MASK;

    // Make sure next capture starts on rising edge
    TIMER3_CTL_R = (TIMER3_CTL_R & ~TIMER3B_EVENT_MASK) | TIMER3B_EVENT_RISING;

    // Disconnect timer from PB3 and use GPIO output
    ping_config_pb3_gpio_output();

    // Send trigger pulse: low -> high for 5us -> low
    GPIO_PORTB_DATA_R &= ~PING_PIN_MASK;
    timer_waitMicros(2);
    GPIO_PORTB_DATA_R |= PING_PIN_MASK;
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~PING_PIN_MASK;

    // Reconnect PB3 to timer capture input
    ping_config_pb3_timer_capture();

    // Reset timer registers for fresh capture
    TIMER3_TBILR_R = 0xFFFF;
    TIMER3_TBPR_R  = 0xFF;

    // Clear interrupt flag
    TIMER3_ICR_R = TIMER3B_INT_MASK;

    // Re-enable interrupt and timer
    TIMER3_IMR_R |= TIMER3B_INT_MASK;
    TIMER3_CTL_R |= TIMER3B_ENABLE_MASK;
}

void TIMER3B_Handler(void)
{
    // Check if this was really Timer3B capture interrupt
    if ((TIMER3_MIS_R & TIMER3B_INT_MASK) == 0)
    {
        return;
    }

    // Clear interrupt
    TIMER3_ICR_R = TIMER3B_INT_MASK;

    if (g_state == LOW)
    {
        // First edge = rising edge, echo pulse started
        g_start_time = TIMER3_TBR_R & 0x00FFFFFF;
        g_state = HIGH;

        // Now wait for falling edge only
        TIMER3_CTL_R = (TIMER3_CTL_R & ~TIMER3B_EVENT_MASK) | TIMER3B_EVENT_FALLING;
    }
    else if (g_state == HIGH)
    {
        // Second edge = falling edge, echo pulse ended
        g_end_time = TIMER3_TBR_R & 0x00FFFFFF;
        g_state = DONE;

        // Stop further capture until next trigger
        TIMER3_IMR_R &= ~TIMER3B_INT_MASK;
        TIMER3_CTL_R &= ~TIMER3B_ENABLE_MASK;
    }
}

float ping_getDistance(void)
{
    uint32_t start_wait;

    ping_trigger();
    start_wait = timer_getMillis();

    // Wait until ISR gets both edges or timeout happens
    while (g_state != DONE)
    {
        if ((timer_getMillis() - start_wait) > 50)
        {
            g_last_pulse_ticks = 0;
            g_last_pulse_ms = 0.0f;
            return -1.0f;   // timeout
        }
    }

    // Since timer is counting DOWN, normal case is start >= end
    if (g_start_time >= g_end_time)
    {
        g_last_pulse_ticks = g_start_time - g_end_time;
        g_last_overflow = false;
    }
    else
    {
        // Timer wrapped around once
        g_last_pulse_ticks = g_start_time + (TIMER3B_24BIT_MAX - g_end_time);
        g_last_overflow = true;
        g_overflow_count++;
    }

    // Convert ticks to milliseconds
    // 16 MHz clock => 16000 ticks per millisecond
    g_last_pulse_ms = ((float)g_last_pulse_ticks) / 16000.0f;

    // Distance in cm
    // speed of sound = 34300 cm/s = 34.3 cm/ms
    // divide by 2 because pulse travels to object and back
    return g_last_pulse_ms * 17.15f;
}
