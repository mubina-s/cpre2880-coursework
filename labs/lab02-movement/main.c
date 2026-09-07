/*
 * main.c
 *
 *  Created on: Feb 12, 2026
 *      Author: mubish
 */

#include "movement.h"
#include "Timer.h"
#include "lcd.h"
#include "open_interface.h"


void main(){

	lcd_init();
	oi_t *sensor_data = oi_alloc();
	oi_init(sensor_data);

	oi_setWheels(0, 0);
	move_forward(sensor_data, 2000);
	lcd_printf("finished!!!!!!");
	oi_free(sensor_data);

}
