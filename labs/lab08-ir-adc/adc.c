/*
 * adc.c
 *
 *  Created on: Mar 27, 2026
 *      Author: mubish
 */

#include "adc.h"
#include <inc/tm4c123gh6pm.h>

typedef struct {
uint16_t raw;
float cm;
} adc_cal_point_t;


static const adc_cal_point_t ir_table[] = {
	{3000, 10.0f},
	{2600, 12.0f},
	{2050, 15.0f},
	{1700, 18.0f},
	{1400, 22.0f},
	{1200, 25.0f},
	{1000, 30.0f},
	{ 850, 35.0f},
	{ 720, 40.0f},
	{ 620, 45.0f},
	{ 540, 50.0f}
};

void adc_init(void){
	// Enable clock for port b
	SYSCTL_RCGCGPIO_R |= 0x02;
	while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}

	// Configuration of the PB4 as analog with the input AIN10 from the tiva datasheet and other resources from the class
	GPIO_PORTB_DIR_R &= ~0x10;
	GPIO_PORTB_AFSEL_R |= 0x10;
	GPIO_PORTB_DEN_R &= ~0x10;
	GPIO_PORTB_AMSEL_R |= 0x10;
	GPIO_PORTB_PCTL_R &= ~0x000F0000;

	// Enable the clock for ADC0
	SYSCTL_RCGCADC_R |= 0x01;
	while ((SYSCTL_PRADC_R & 0x01) == 0) {};

	ADC0_ACTSS_R &= ~0x08;

	ADC0_EMUX_R &= ~0xF000;

	ADC0_SSMUX3_R = 10;

	ADC0_SSCTL3_R = 0x0006;

	ADC0_IM_R &= ~0x08;

	ADC0_ISC_R = 0x08;

	ADC0_ACTSS_R |= 0x08;
}

uint16_t adc_read(void){
	uint16_t result;

	ADC0_PSSI_R = 0x08;
	while((ADC0_RIS_R & 0x08) == 0) {}

	result = ADC0_SSFIFO3_R & 0x0FFF;
	ADC0_ISC_R = 0x08;

	return result;
}
