/*
 * movement.h
 *
 *  Created on: Feb 12, 2026
 *      Author: mubish
 */

#ifndef MOVEMENT_H_
#define MOVEMENT_H_

#include "open_interface.h"
#include "Timer.h"
#include "lcd.h"

void move_forward(oi_t *sensor_data, double travel_distance);
void turn_left(oi_t *sensor_data, double degrees);
void turn_right(oi_t *sensor_data, double degrees);
void move_backward(oi_t *sensor_data, double travel_distance);
void bumperRight(oi_t *sensor_data, double distance_remaining);
void bumperLeft(oi_t *sensor_data, double distance_remaining);



#endif /* MOVEMENT_H_ */
