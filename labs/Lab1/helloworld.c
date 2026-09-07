/// Simple 'Hello, world' program
/**
 * This program prints "Hello, world" to the LCD screen
 * @author Chad Nelson
 * @date 06/26/2012
 *
 * updated: phjones 9/3/2019
 * Description: Added timer_init call, and including Timer.h
 */

#include "Timer.h"
#include "lcd.h"
#include <string.h>

char charsDisplayed[21] = "                    ";
int i = 0;
int j = 20;
int k = 0;

void remove_first_character( char *str);
void lcd_rotatingBanner(char data[]);

int main (void) {

	timer_init(); // Initialize Timer, needed before any LCD screen functions can be called 
	              // and enables time functions (e.g. timer_waitMillis)

	lcd_init();   // Initialize the LCD screen.  This also clears the screen. 

	// Print "Hello, world" on the LCD
	lcd_rotatingBanner("Microcontrollers are lots of fun!");

	// lcd_puts("Hello, world"); // Replace lcd_printf with lcd_puts
        // step through in debug mode and explain to TA how it works
    
	// NOTE: It is recommended that you use only lcd_init(), lcd_printf(), lcd_putc, and lcd_puts from lcd.h.
       // NOTE: For time functions, see Timer.h

	return 0;
}

void remove_first_character(char *str){
	for (i = 0; i < 19; i++){
		str[i] = str[i+1];
	}
	str[19] = ' ';
	str[20] = '\0';
}

void lcd_rotatingBanner(char data[]){
	while (1) {
		lcd_init();
		/*for(i = 0; i<20; i++){
			lcd_putc(data[i]);
			charsDisplayed[i] = data[i]; //strcat(charsDisplayed, data[i]);
			timer_waitMillis(300);
		} */
		for(j = 0; j<strlen(data); j++){
			remove_first_character(charsDisplayed);
			lcd_init();
			charsDisplayed[19] = data[j]; //strcat(charsDisplayed, data[j]);
			lcd_printf("%s", charsDisplayed);
			timer_waitMillis(300);
		}
		for(k = 0; k<19; k++){
			remove_first_character(charsDisplayed);
			charsDisplayed[19] = ' ';
			charsDisplayed[20] = '\0';
			lcd_printf("%s", charsDisplayed);
			timer_waitMillis(300);
		}
	}
}
