#ifndef PING_H_
#define PING_H_

#include <stdint.h>
#include <stdbool.h>
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"

// Global shared variables
extern volatile uint32_t g_start_time;
extern volatile uint32_t g_end_time;
extern volatile uint32_t g_last_pulse_ticks;
extern volatile uint32_t g_overflow_count;
extern volatile bool g_last_overflow;
extern volatile float g_last_pulse_ms;

void ping_init(void);
void ping_trigger(void);
void TIMER3B_Handler(void);
float ping_getDistance(void);

#endif /* PING_H_ */
