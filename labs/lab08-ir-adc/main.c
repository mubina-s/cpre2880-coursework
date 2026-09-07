#include "Timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"
#include "cyBot_Scan.h"
#include "uart-interrupt.h"
#include <stdio.h>
#include <stdint.h>
#include "driverlib/interrupt.h"
#include "adc.h"


uint16_t median3U16Array(uint16_t vals[3])
{
    uint32_t sum = vals[0] + vals[1] + vals[2];
    uint16_t min = vals[0];
    uint16_t max = vals[0];

    if (vals[1] < min) min = vals[1];
    if (vals[2] < min) min = vals[2];

    if (vals[1] > max) max = vals[1];
    if (vals[2] > max) max = vals[2];

    return (uint16_t)(sum - min - max);
}
double median3DoubleArray(double vals[3])
{
    double sum = vals[0] + vals[1] + vals[2];
    double min = vals[0];
    double max = vals[0];

    if (vals[1] < min) min = vals[1];
    if (vals[2] < min) min = vals[2];

    if (vals[1] > max) max = vals[1];
    if (vals[2] > max) max = vals[2];

    return sum - min - max;
}

double rawToDistIR(double raw){
	return 1.0543712208e7 * pow(raw, -1.7446467698);
}

//typedef struct{
//	uint16_t adc;
//	double dist;
//} ir_cal_t;
//
//ir_cal_t ir_table[] = {
// {3273, 9.2},
// {3056, 9.8},
// {2975, 10.3},
// {2504, 12.3},
// {2386, 14.1},
// {2247, 15.5},
// {2196, 15.7},
// {2117, 17.1},
// {2030, 18.8},
// {1920, 20.0},
// {1823, 22.0},
// {1715, 23.2},
// {1645, 25.4},
// {1583, 27.4},
// {1481, 29.4},
// {1448, 30.8},
// {1417, 32.3},
// {1377, 34.2},
// {1300, 36.2},
// {1262, 40.0},
// {1240, 41.5},
// {1220, 43.5},
// {1202, 45.5},
// {1201, 46.4},
// {1180, 48.5},
//};
//
//double rawToDistIR_table(uint16_t raw){
//	int i;
//	int tableSize = sizeof(ir_table) / sizeof(ir_table[0]);
//
//	for(i = 0; i<tableSize - 1; i++){
//		double adc1 = ir_table[i].adc;
//		double adc2 = ir_table[i+1].adc;
//		double dist1 = ir_table[i].dist;
//		double dist2 = ir_table[i+1].dist;
//
//		if (raw <= adc1 && raw >= adc2){
//			return dist1 + (((double) raw - adc1) * (dist2 - dist1) / (adc2 - adc1));
//		}
//	}
//	return -1.0;
//}




void getCalibrationSample(short angle, uint16_t *adcMed, double *pingMed, double *irMed)
{
    cyBOT_Scan_t scan;
    uint16_t adcVals[3];
    double pingVals[3];
//    double irVals[3];
    int i;

    for (i = 0; i < 3; i++)
    {
        cyBOT_Scan(angle, &scan);
        adcVals[i] = adc_read();
        pingVals[i] = scan.sound_dist;
//        irVals[i] = scan.IR_raw_val;
    }

    *adcMed = median3U16Array(adcVals);
    *pingMed = median3DoubleArray(pingVals) - 1.5;
    *irMed = rawToDistIR(*adcMed);
//    *rawirMed = median3DoubleArray(irVals);
}


int main(void)
{
    timer_init();
    lcd_init();
    uart_interrupt_init();
    adc_init();
    cyBOT_init_Scan(0b0111);

    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);

//	cyBOT_SERVO_cal();
//	cyBOT_SERVRO_cal_t servo = cyBOT_SERVO_cal();
//	left_calibration_value = servo.left;
//	right_calibration_value = servo.right;  //used for calibrating the sensor data
    right_calibration_value = 238000;
    left_calibration_value = 1188250;

    char buffer[128];

    uint16_t adcMed = 0;
    double pingMed = 0.0;
    double irMed = 0.0;
//    double rawirMed = 0.0;

    short angle = 90;
//    move_forward(sensor_data, 2);
    uart_sendStr("press n to scan");

//    command_byte = 'n';
//    command_flag = 0;

    while (1) {


//        if (command_flag) {
//
//        	command_flag = 0;

            getCalibrationSample(angle, &adcMed, &pingMed, &irMed);

            sprintf(buffer, "\r\nadc: %u\r\nping: %.2f\r\nir dist:%0.1f\r\n{%u, %.1f}\r\n", adcMed, pingMed, irMed, adcMed, pingMed);
            uart_sendStr(buffer);

            lcd_printf("adc:%u\nping:%.2f\nir:%.1f", adcMed, pingMed, irMed);
            timer_waitMillis(500);
//        }

    }
}
