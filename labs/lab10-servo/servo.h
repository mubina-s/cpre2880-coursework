#ifndef SERVO_H_
#define SERVO_H_

#include <stdint.h>
#include <inc/tm4c123gh6pm.h>

void servo_init(void);
void servo_move(uint16_t degree);
uint16_t servo_get_degree(void);
uint32_t servo_get_match(void);

#endif
