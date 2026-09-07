/**
 * lab4_template.c
 *
 * Template file for CprE 288 lab 4
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 */

#include "cyBot_uart.h"
#include "button.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"  // Functions for communicating between CyBot and Putty (via UART)
                         // PuTTy: Baud=115200, 8 data bits, No Flow Control, No Parity, COM1

// #warning "Possible unimplemented functions"
// #define REPLACEME 0

void uart_sendString(char s[]);

int main(void){
	timer_init();
	lcd_init();
	button_init();
	cyBot_uart_init();

	char msg[35];
	int button;

	while(1){
		button = button_getButton();


		if((button == 0)){
			lcd_printf("Button non pressed");
		}else{
			sprintf(msg, "\r\nButton pressed is %d", button);
			lcd_printf(msg);
			uart_sendString(msg);
		}

		timer_waitMillis(150);
	}
}

void uart_sendString(char* s){
	int i;
	for (i = 0; i < strlen(s);i++){
		cyBot_sendByte(s[i]);
	}
}
