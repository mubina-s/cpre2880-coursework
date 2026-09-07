#include <stdint.h>
#include <inc/tm4c123gh6pm.h>
#include "lcd.h"
#include "Timer.h"
#include "servo.h"

int main(void)
{
    timer_init();
    lcd_init();
    servo_init();

    lcd_printf("start 90");
    servo_move(90);
    timer_waitMillis(1500);

    lcd_printf("move 30\nmatch:%u", servo_get_match());
    servo_move(30);
    timer_waitMillis(1500);

    lcd_printf("move 150\nmatch:%u", servo_get_match());
    servo_move(150);
    timer_waitMillis(1500);

    lcd_printf("back to 90\nmatch:%u", servo_get_match());
    servo_move(90);
    timer_waitMillis(1500);

    lcd_printf("holding 90\nmatch:%u", servo_get_match());

    while (1)
    {
        // hold at 90
    }
}
