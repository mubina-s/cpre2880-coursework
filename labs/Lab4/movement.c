/*
 * movement.c
 *
 *  Created on: Feb 12, 2026
 *      Author: mubish
 */

#include "open_interface.h"
#include "movement.h"
#include "Timer.h"
#include "lcd.h"

void move_forward(oi_t *sensor_data, double travel_distance);
void turn_left(oi_t *sensor_data, double degrees);
void turn_right(oi_t *sensor_data, double degrees);
void move_backward(oi_t *sensor_data, double travel_distance);
void bumperRight(oi_t *sensor_data, double distance_remaining);
void bumperLeft(oi_t *sensor_data, double distance_remaining);

void move_forward(oi_t *sensor_data, double travel_distance){
	double total_mm = 0;
	/*lcd_printf("distance remaining: %4.0f", travel_distance);
	timer_waitMillis(2000); */
	oi_setWheels(200,200);
	while (total_mm<travel_distance){
		oi_update(sensor_data);
		if(sensor_data->bumpLeft){
			oi_setWheels(0,0);
			// timer_waitMillis(2000);
			bumperLeft(sensor_data, (travel_distance-total_mm));
		}
		if(sensor_data->bumpRight){
			oi_setWheels(0,0);
			// timer_waitMillis(2000);
			bumperRight(sensor_data, (travel_distance-total_mm));
		}
		total_mm += sensor_data->distance;
		lcd_printf("Distance: %4.0f mm", total_mm);
	}
	oi_setWheels(0,0);
}

void move_backward(oi_t *sensor_data, double travel_distance){
	double total_mm = 0;
	/*lcd_printf("distance remaining: %4.0f", travel_distance);
	timer_waitMillis(2000); */
	oi_setWheels(-200,-200);
	while (total_mm<travel_distance){
		oi_update(sensor_data);
		/*if(sensor_data->bumpLeft){
			oi_setWheels(0,0);
			timer_waitMillis(2000);
			bumperLeft(sensor_data, (travel_distance-total_mm));
		}
		if(sensor_data->bumpRight){
			oi_setWheels(0,0);
			timer_waitMillis(2000);
			bumperRight(sensor_data, (travel_distance-total_mm));
		} */
		total_mm -= sensor_data->distance;
		lcd_printf("Distance: %4.0f mm", total_mm);
	}
	oi_setWheels(0,0);
}

void turn_left(oi_t *sensor_data, double degrees){
	double degrees_turned = 0;
	oi_setWheels(50,-50);
	while (degrees_turned < (degrees/1.05)){
		oi_update(sensor_data);
		degrees_turned += sensor_data->angle;
		lcd_printf("Angle: %4.0f degrees", (degrees_turned*1.05));
	}
	oi_setWheels(0, 0);
}

void turn_right(oi_t *sensor_data, double degrees){
	double degrees_turned = 0;
	oi_setWheels(-50,50);
	while (degrees_turned < (degrees/1.05)){
		oi_update(sensor_data);
		degrees_turned -= sensor_data->angle;
		lcd_printf("Angle: %4.0f degrees", (degrees_turned*1.05));
	}
	oi_setWheels(0, 0);
}

void bumperRight(oi_t *sensor_data, double distance_remaining){
	oi_setWheels(0,0);
	/*lcd_printf("remaining: %4.0f", distance_remaining);
	timer_waitMillis(2000); */
	move_backward(sensor_data, 150);
	turn_left(sensor_data, 90);
	move_forward(sensor_data, 250);
	turn_right(sensor_data, 90);
	move_forward(sensor_data, distance_remaining+150);
}

void bumperLeft(oi_t *sensor_data, double distance_remaining){
	oi_setWheels(0,0);
	/*lcd_printf("remaining: %4.0f", distance_remaining);
	timer_waitMillis(2000); */
	move_backward(sensor_data, 150);
	turn_left(sensor_data, 90);
	move_forward(sensor_data, 250);
	turn_right(sensor_data, 90);
	move_forward(sensor_data, distance_remaining+150);
}
