/**
 * lab5_template.c
 *
 * Template file for CprE 288 Lab 5
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 *
 * @author Phillip Jones, updated 6/4/2019
 * @author Diane Rover, updated 2/25/2021, 2/17/2022
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"

#include "cyBot_uart.h"  // Functions for communicating between CyBot and Putty (via UART1)
                         // PuTTy: Baud=115200, 8 data bits, No Flow Control, No Parity, COM1

#include "cyBot_Scan.h"  // Scan using CyBot servo and sensors
#include "uart.h"


//#warning "Possible unimplemented functions"
//#define REPLACEME 0



int main(void) {
	button_init();
	timer_init(); // Must be called before lcd_init(), which uses timer functions
	lcd_init();
    uart_init();
	
	char recievedByte;
	char buffer[21];
	int count = 0;
	char firstChar;
	char secondChar;
	int i;

  // initialize the cyBot UART1 before trying to use it

//   //(Uncomment ME for UART init part of lab)
//	 cyBot_uart_init_clean();  // Clean UART1 initialization, before running your UART1 GPIO init code
//
////	// Complete this code for configuring the GPIO PORTB part of UART1 initialization (your UART1 GPIO init code)
//      SYSCTL_RCGCGPIO_R |= 0x02;
//	    while ((SYSCTL_PRGPIO_R & 0x02) == 0) {};
//		  GPIO_PORTB_DEN_R |= 0x03;
//		  GPIO_PORTB_AFSEL_R |= 0x03;
//      GPIO_PORTB_PCTL_R &= ~0x000000FF;     // Force 0's in the desired locations
//      GPIO_PORTB_PCTL_R |= 0x00000011;     // Force 1's in the desired locations
//		 // Or see the notes for a coding alternative to assign a value to the PCTL field
//
//     //(Uncomment ME for UART init part of lab)
//		 cyBot_uart_init_last_half();  // Complete the UART device configuration

//		//Initialize the scan
//		cyBOT_init_Scan(0b0111);
//		//Remember servo calibration function and variables from Lab 3
//		cyBOT_SERVO_cal();
//		cyBOT_SERVRO_cal_t servo = cyBOT_SERVO_cal();
//		left_calibration_value = servo.left;
//		right_calibration_value = servo.right;  //used for calibrating the sensor data
//		//right_calibration_value = 337750;
//		//left_calibration_value = 1234600;

	// YOUR CODE HERE


	while(1)
	{

      // YOUR CODE HERE
	    recievedByte = uart_receive();
	    if(recievedByte == '\r'){
	        lcd_clear();
	        lcd_gotoLine(1);
	        lcd_puts(buffer);
	        lcd_gotoLine(2);
	        firstChar = (count/10) + '0';
	        lcd_putc(firstChar);
	        secondChar = (count%10) + '0';
	        lcd_putc(secondChar);
	    } else{
	        if (count < 19){
	            buffer[count] = recievedByte;
	            count++;
	            uart_sendChar(recievedByte);
	        } else if (count == 19){
	            lcd_clear();
	            uart_sendChar(recievedByte);
	            buffer[count] = recievedByte;
	            count++;
	            lcd_gotoLine(1);
	            lcd_puts(buffer);
	            lcd_gotoLine(2);
	            firstChar = (count/10) + '0';
	            lcd_putc(firstChar);
	            secondChar = (count%10) + '0';
	            lcd_putc(secondChar);
	            count = 0;
	            for (i = 0; i < sizeof(buffer); i++){
	                buffer[i] = '\0';
	            }
	        }
	    }

	}

}
/*
void uart_sendString(char* s){
	int i;
	for (i = 0; i < strlen(s);i++){
		cyBot_sendByte(s[i]);
	}
}
*/
