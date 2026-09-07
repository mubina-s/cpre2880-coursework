/*
 * main.c
 *
 *  Created on: Feb 12, 2026
 *      Author: mubish
 */


#include "cyBot_uart.h"
#include "lcd.h"
#include "open_interface.h"
#include "cyBot_Scan.h"
#include "Timer.h"
#include "movement.h"
#include <stdio.h>
#include <string.h>

void uart_sendString(char s[]);

/* struct objectData{
		int objectNum[10];
		int startAngle[10];
		int middleAngle[10];
		int endAngle[10];
		int distance_cm[10];
		int radialWidth[10];
	};
	*/

typedef struct{
	int objectNum;
	int startAngle;
	int middleAngle;
	int endAngle;
	int distance_cm;
	int radialWidth;
} objectData_t;

int main(void){
	char recievedByte;
	char buffer[50];
	int angle;
	int objectNum = 0;
	int toggle = 0;
	int prevScan;
	int i;
	int smallestObjAng = 0;
	timer_init();
	lcd_init();

	cyBot_uart_init();
	cyBOT_init_Scan(0b0111);

	oi_t *sensor_data = oi_alloc();
	oi_init(sensor_data);

	//cyBOT_SERVO_cal();
	//cyBOT_SERVRO_cal_t servo = cyBOT_SERVO_cal();
	//left_calibration_value = servo.left;
	//right_calibration_value = servo.right;  //used for calibrating the sensor data
	 right_calibration_value = 337750;
	 left_calibration_value = 1234600;

	while(1){
		recievedByte = cyBot_getByte();
	        if (recievedByte == 'm'){
	            cyBOT_Scan_t scan;

	            // \n is a new line, r mooves the cursor to the beginning of the SAME line, t adds a big space or column space
	            // \r\n go to new line, \t separates columns
	            uart_sendString("\r\n Angle \t Ping(CM) \r\n");
	            objectData_t objectList[5];
	            for(angle = 0; angle <= 180; angle += 2){ //Got from the Servo Calibration Reference Sheet just cahnged angles
	                cyBOT_Scan(angle, &scan);
	                if (scan.sound_dist < 100 && toggle == 0 && angle > 6){
	                	objectList[objectNum].objectNum = objectNum;
	                	objectList[objectNum].distance_cm = scan.sound_dist;
	                	objectList[objectNum].startAngle = angle;
	                	toggle = 1;
	                }
	                if (scan.sound_dist > 100 && toggle == 1){
	                	objectList[objectNum].endAngle = prevScan;
	                	objectList[objectNum].middleAngle = ((prevScan - objectList[objectNum].startAngle)/2)+objectList[objectNum].startAngle;
	                	objectList[objectNum].radialWidth = prevScan - objectList[objectNum].startAngle;
	                	objectNum++;
	                	toggle = 0;
	                }
	                sprintf(buffer, "%d \t %.2f \r\n", angle, scan.sound_dist);

	                uart_sendString(buffer);

	                prevScan = angle;
	            }
	            smallestObjAng = objectList[0].middleAngle;
	            for (i = 0; i < objectNum; i++){
	            	if (objectList[i+1].radialWidth < objectList[i+1].radialWidth){
	            		smallestObjAng = objectList[i+1].middleAngle;
	            	}
	            }
	            cyBOT_Scan(smallestObjAng, &scan);
	            //part 4
	            /*if (smallestObjAng < 90.0){
	            	turn_right(sensor_data, (90.0 - smallestObjAng));
	            } else if(smallestObjAng > 90.0){
	            	turn_left(sensor_data, (180.0 - smallestObjAng));
	            }
	            move_forward(sensor_data, ((scan.sound_dist*10.0) - 100.0)); */
	            uart_sendString("\r\nobjectNum startAngle middleAngle endAngle distance radialWidth\r\n");
	            for (i = 0; i < objectNum; i++){
	            	sprintf(buffer, "\r\n%d        %d         %d        %d       %d       %d", objectList[i].objectNum, objectList[i].startAngle,
	            			objectList[i].middleAngle, objectList[i].endAngle, objectList[i].distance_cm, objectList[i].radialWidth);
	            	uart_sendString(buffer);
	            }
	            objectNum = 0;
	        }
	    }

	/* while(1){
		recievedByte = cyBot_getByte();

		lcd_clear();
		//lcd_puts("got a ");
		lcd_putc(recievedByte);
		sprintf(buffer, "%c", recievedByte);
		uart_sendString("Got a ");
		uart_sendString(buffer);
	}
*/
}


void uart_sendString(char* s){
	int i;
	for (i = 0; i < strlen(s);i++){
		cyBot_sendByte(s[i]);
	}
}

// void cyBOT_Scan(int angle, cyBOT_Scan_t* getScan);
