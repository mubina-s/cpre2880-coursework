#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "Timer.h"
#include "servo.h"

uint16_t current_degree = 90;

uint32_t degree_to_high(uint16_t degree) {
    uint32_t high;

    if (degree > 180) {
        degree = 180;
    }

    // 1 ms = 16000
    // 2 ms = 32000
    high = 16000 + ((32000 - 16000) * degree) / 180;

    return high;
}

uint32_t degree_to_match(uint16_t degree)
{
    uint32_t high;
    uint32_t low;
    uint32_t match;

    high = degree_to_high(degree);

    // period is 20 ms = 320000 counts
    low = 320000 - high;

    match = low - 1;

    return match;
}

void servo_init(void) {
    uint32_t match;

    // turn on port b clock
    SYSCTL_RCGCGPIO_R |= 0x02;

    // turn on timer 1 clock
    SYSCTL_RCGCTIMER_R |= 0x02;

    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
    while ((SYSCTL_PRTIMER_R & 0x02) == 0){}

    GPIO_PORTB_AFSEL_R |= 0x20;
    GPIO_PORTB_DEN_R |= 0x20;
    GPIO_PORTB_DIR_R |= 0x20;
    GPIO_PORTB_AMSEL_R &= ~0x20;

    GPIO_PORTB_PCTL_R &= ~0x00F00000;
    GPIO_PORTB_PCTL_R |= 0x00700000;

    // turn off timer1b
    TIMER1_CTL_R &= ~0x0100;
    TIMER1_CFG_R = 0x04;

    //timer b periodic pwm mode
    TIMER1_TBMR_R = 0x0A;
    TIMER1_CTL_R &= ~0x4000;

    TIMER1_TBILR_R = 319999 & 0xFFFF;
    TIMER1_TBPR_R = (319999 >> 16) & 0xFF;

    //start at 90 degrees
    match = degree_to_match(90);
    TIMER1_TBMATCHR_R = match & 0xFFFF;
    TIMER1_TBPMR_R = (match >> 16) & 0xFF;

    //timer back on
    TIMER1_CTL_R |= 0x0100;

    current_degree = 90;

    timer_waitMillis(500);
}

void servo_move(uint16_t degree) {
    uint32_t match;

    if (degree > 180) {
        degree = 180;
    }

    match = degree_to_match(degree);

    TIMER1_TBMATCHR_R = match & 0xFFFF;
    TIMER1_TBPMR_R = (match >> 16) & 0xFF;

    current_degree = degree;
}

uint16_t servo_get_degree(void) {
    return current_degree;
}

uint32_t servo_get_match(void) {
    uint32_t match;

    match = 0;
    match |= ((TIMER1_TBPMR_R & 0xFF) << 16);
    match |= (TIMER1_TBMATCHR_R & 0xFFFF);

    return match;
}
