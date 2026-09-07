/**
 * lab6-interrupt_template.c
 *
 * Template file for CprE 288 Lab 6
 *
 * @author Diane Rover, 2/15/2020
 *
 */

#include "Timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"  // For scan sensors
#include "uart-interrupt.h"
#include <stdio.h>
#include <stdbool.h>
#include "driverlib/interrupt.h"

// Uncomment or add any include directives that you want to use
// #include "open_interface.h"
// #include "movement.h"
// #include "button.h"

// Your code can use the global variables defined in uart-interrupt.c
// They are declared with the extern qualifier in uart-interrupt.h, which makes the variables visible to this file.

#warning "Possible unimplemented functions"
#define REPLACEME 0


int main(void) {
	timer_init(); // Must be called before lcd_init(), which uses timer functions
	lcd_init();
	uart_interrupt_init();
	cyBOT_init_Scan(0b0111);

	char c;
	char s;
	char msg[80];
	int angle;

	uart_sendStr("Lab06 Part01\r\n");
	uart_sendStr("Type shiiii \r\n.");
	cyBOT_SERVO_cal();
	cyBOT_SERVRO_cal_t servo = cyBOT_SERVO_cal();
	left_calibration_value = servo.left;
	right_calibration_value = servo.right;  //used for calibrating the sensor data
//	right_calibration_value = 353500;
//	left_calibration_value = 1314250;

	cyBOT_Scan_t scan;
	command_byte = 's';
	command_flag = 0;
	// OPTIONAL
	//assign a value to command_byte if you want to know whether that ASCII code is received
	//note that command_byte is global shared variable read by the ISR
	//for example, try using a tab character as a command from PuTTY

	while(1)
	{

		 // waits for "g" to start, which is blocking ok here because we are idle
		        c = uart_receive();
		        uart_sendChar(c);

		        if (c == 'g')
		        {
		            uart_sendStr("Starting scan... (press 's' to stop)\r\n");

		            // scan loop
		            for (angle = 0; angle <= 180; angle += 2)
		            {
		                // check stop command without blocking
		            	s = uart_receive_nonblocking();


		                if (command_flag == 1)
		                {
		                    uart_sendStr("Scan stopped!\r\n");
		                    command_flag = 0;
		                    break;
		                }

		                cyBOT_Scan(angle, &scan);

		                // send scan data for the angle, ping dist, ir raw
		                // sound_dist is float, cm, IR_raw_val is int


		                snprintf(msg, sizeof(msg), "%d, %.2f, %d\r\n",
		                         angle, scan.sound_dist, scan.IR_raw_val);
		                uart_sendStr(msg);

		                timer_waitMillis(50);
		            }

		            uart_sendStr("Done.\r\n");
		            uart_sendStr("Type 'g' to scan again.\r\n");
		        }
//		if(command_flag == 1){
//			command_flag = 0;
//			uart_sendStr("\r\nCmmand recieed");
//			lcd_clear();
//			lcd_puts("command seen");
//		}

	}
}

      // YOUR CODE HERE
			//first, try leaving this loop empty and see what happens
			//then add code for your application
			// OPTIONAL
			//test and reset command_flag if your ISR is updating it
			//for example, if the flag is 1, do something, like send a message to PuTTY or LCD, or stop a sensor scan, etc.
			//be sure to reset command_flag so you don't keep responding to an old flag

