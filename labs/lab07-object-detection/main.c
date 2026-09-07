#include "Timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"
#include "cyBot_Scan.h"
#include "uart-interrupt.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/interrupt.h"
#include <math.h>


extern int rescan_needed;

typedef struct{
    int objectNum;
    int startAngle;
    int middleAngle;
    int endAngle;
    int distance_cm;
    int theta;
    int linearWidth;
} objectData_t;


//typedef struct{
//    int angle;
//    int dist_ping;
//    int dist_IR;
//} sensor_data_t;


typedef struct{
    int num;
    int width;
    int dist;
} objects_t;

//this is for cyBOT 15

double median3(double med3[3]){
	double sum = med3[0] + med3[1] + med3[2];
	double min = med3[0];
	if (med3[1] < min) min = med3[1];
	if (med3[2] < min) min = med3[2];
	double max = med3[0];
	if (med3[1] > max) max = med3[1];
	if (med3[2] > max) max = med3[2];
	return (sum - max - min);
}
int median3Int(int med3[3]){
	int sum = med3[0] + med3[1] + med3[2];
	int min = med3[0];
	if (med3[1] < min) min = med3[1];
	if (med3[2] < min) min = med3[2];
	int max = med3[0];
	if (med3[1] > max) max = med3[1];
	if (med3[2] > max) max = med3[2];
	return (sum - max - min);
}

int rawToDistIR(int raw){
	return (int)(420.21*exp(-0.002 * raw));
}
//(342.14*exp(-0.002 * raw)    cyBOT 15
void averagedMean(int *IRDist, int *pingDist, cyBOT_Scan_t *scan, int angle){

    int medIR[3];
	double medPing[3];
    int i;
    for(i = 0; i < 3; i++){
        cyBOT_Scan(angle, scan);
        medIR[i] = scan->IR_raw_val;
        medPing[i] = scan->sound_dist;
    }
    *IRDist = rawToDistIR(median3Int(medIR));
    *pingDist = median3(medPing);
}

int main(void) {
    timer_init();
    lcd_init();
    uart_interrupt_init();
    cyBOT_init_Scan(0b0111);

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

    char c;
    char buffer[120];
    short angle;
    double averagePing;
    int averageIR;
    int med3[3];
    double medi3[3];

    int averagederrerIR = 0;
    double averagederrerPing = 0;
//    move_forward(sensor_data, 10);
//    turn_right(sensor_data, 90);
    int i = 0;
    int j = 0;
    int toggle = 0;
    int objectNum = 0;
    double prevScan = 0;
    int smallestObject = 0;
    int scanThreshold = 100;
	int IRDist = 0;
	int pingDist = 0;

//	cyBOT_SERVO_cal();
//	cyBOT_SERVRO_cal_t servo = cyBOT_SERVO_cal();
//	left_calibration_value = servo.left;
//	right_calibration_value = servo.right;  //used for calibrating the sensor data
	right_calibration_value = 238000;
	left_calibration_value = 1188250;

    command_byte = 's';
    command_flag = 0;

	while (1) {
		c = uart_receive();
		uart_sendChar(c);

		if (c == 'm') {
			int keepScanning = 1;
			while (keepScanning == 1) {
				objectData_t objectList[10];
				cyBOT_Scan_t scan;
				objectNum = 0;
				toggle = 0;
				prevScan = 0;
				smallestObject = 0;
				IRDist = 0;
				pingDist = 0;
				rescan_needed = 0;

				// setup info
				sprintf(buffer, "\r\nUsing scan threshold: %d", scanThreshold);
				uart_sendStr(buffer);
				uart_sendStr("\r\nDegrees\tPing(cm)\tIR Value(cm)");

				// full scan loop
				for (angle = 0; angle <= 180; angle += 2) {

					averagedMean(&IRDist, &pingDist, &scan, angle + 2);

					if (IRDist < scanThreshold && toggle == 0) {
						toggle = 1;
						objectList[objectNum].startAngle = angle;
					}
					else if (IRDist > scanThreshold + 8 && toggle == 1) { //+8 because it is more resistant to bad data
						objectList[objectNum].endAngle = prevScan;
						objectList[objectNum].middleAngle =
								(objectList[objectNum].startAngle
										+ objectList[objectNum].endAngle) / 2;

						averagePing = 0;

						objectList[objectNum].distance_cm = averagePing / 2;
						objectList[objectNum].theta = objectList[objectNum].endAngle - objectList[objectNum].startAngle;
//                    objectList[objectNum].linearWidth = 2.0 * objectList[objectNum].distance_cm * sin((objectList[objectNum].theta * M_PI / 180.0) / 2.0);
						objectList[objectNum].objectNum = objectNum;
						toggle = 0;
						objectNum++;
					}

					sprintf(buffer, "\r\n%d\t%d\t%d", angle, pingDist, IRDist);
					uart_sendStr(buffer);
					prevScan = angle;
				}

				// if scan ends and toggle still 1 (object is at the end of a scan)
				if (toggle == 1) {
//					objectList[objectNum].endAngle = prevScan;
//					objectList[objectNum].middleAngle =
//							(objectList[objectNum].startAngle + objectList[objectNum].endAngle) / 2.0;
//					objectList[objectNum].theta = objectList[objectNum].endAngle - objectList[objectNum].startAngle;
//					objectList[objectNum].objectNum = objectNum;
					toggle = 0;
//					objectNum++;
				}

				//scan middle of each object
				for (i = 0; i < objectNum; i++) {
					for (j = 0; j < 3; j++) {
						averagedMean(&IRDist, &pingDist, &scan, objectList[i].middleAngle);
						med3[j] = IRDist;
						medi3[j] = pingDist;
					}
					//find median
					averagePing = median3(medi3);
					averageIR = median3Int(med3);
					sprintf(buffer,"\r\nObjNum: %d, Angle: %d, Ping: %0.2f, IR: %d", i, objectList[i].middleAngle, averagePing, averageIR);
					uart_sendStr(buffer);
					objectList[i].distance_cm = averageIR;
					objectList[i].linearWidth = 2.0 * objectList[i].distance_cm* sin((objectList[i].theta * M_PI / 180.0) / 2.0);
				}

				// Print the list objects
				uart_sendStr("\r\nobject \t start \t mid \t end \t dist \t width");
				for (i = 0; i < objectNum; i++) {
					sprintf(buffer, "\r\n%d \t %d \t %d \t %d \t %d \t %d", (i), objectList[i].startAngle, objectList[i].middleAngle,
					objectList[i].endAngle, objectList[i].distance_cm, objectList[i].linearWidth);
					uart_sendStr(buffer);
				}

				// If the number of objects is greater than 0, find the smallest one
				if (objectNum > 0) {
					smallestObject = 0;
					// There is more than one object in the field, avoid potential issues.
					if (objectNum > 1) {
						for (i = 1; i < objectNum; i++) {
							if (objectList[i].linearWidth < objectList[smallestObject].linearWidth) {
								smallestObject = i;
							}
						}
					}

					// Print the smallest object info.
					sprintf(buffer, "\r\nSmallest object = %d, width = %d cm, angle = %d, dist = %d cm\r\n", smallestObject,
					objectList[smallestObject].linearWidth, objectList[smallestObject].middleAngle, objectList[smallestObject].distance_cm);
					uart_sendStr(buffer);

					if (objectList[smallestObject].middleAngle > 90.0) {
						turn_left(sensor_data, objectList[smallestObject].middleAngle - 90.0);
					} else if (objectList[smallestObject].middleAngle < 90.0) {
						turn_right(sensor_data, 90.0 - objectList[smallestObject].middleAngle);
					}

					double remainingDist = objectList[smallestObject].distance_cm - 5.0;
					if (remainingDist > 0) {
						move_forward(sensor_data, remainingDist * 10.0);
					}
				}
				if(rescan_needed == 0){
					keepScanning = 0;
				} else {
					uart_sendStr("\r\nbumper hit, rescanning");
				}
			}
		}
        if(c == '.'){
        	turn_right(sensor_data, 90);
            cyBOT_Scan_t scan;
            for(i = 0; i < 4; i++){
                cyBOT_Scan(90, &scan);
                sprintf(buffer, "\r\nIR: %d, Ping: %0.2f", scan.IR_raw_val, scan.sound_dist);
                uart_sendStr(buffer);
                averagederrerIR += scan.IR_raw_val;
                averagederrerPing += scan.sound_dist;
            }
            sprintf(buffer, "\r\nAvgIR: %d, AvgPing: %0.2f", (averagederrerIR / 4), (averagederrerPing / 4));
            uart_sendStr(buffer);
            averagederrerIR = 0;
            averagederrerPing = 0;
        }
    }
}

//for ping value and ping distance
//int main(void) {
//    timer_init();
//    lcd_init();
//    uart_interrupt_init();
//    cyBOT_init_Scan(0b0111);
//
//    oi_t *sensor_data = oi_alloc();
//    oi_init(sensor_data);
//    cyBOT_Scan_t scan;
//
////  cyBOT_SERVO_cal();
////	cyBOT_SERVRO_cal_t servo = cyBOT_SERVO_cal();
////	left_calibration_value = servo.left;
////	right_calibration_value = servo.right;  //used for calibrating the sensor data
//    right_calibration_value = 269500;
//    left_calibration_value = 1204000;
//
//    int IR_value;
//    int Ping_dist;
//
//    while(1){
//    	cyBOT_Scan(90, &scan);
//        IR_value = scan.IR_raw_val;
//        Ping_dist = scan.sound_dist;
//
//        int y;
//        y = (0.00002 * (IR_value * IR_value)) - (0.0933 * IR_value) + 130.72;
//
//
//        lcd_printf("IR dist: %d \n Ping dist: %d", y, Ping_dist);
//        timer_waitMillis(500);
////y = 2E-05x^2 - 0.0933x + 130.72
//
//	}
//}
