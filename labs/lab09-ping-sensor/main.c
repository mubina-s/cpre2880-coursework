#include "Timer.h"
#include "lcd.h"
#include "ping.h"

int main(void)
{
    timer_init();
    lcd_init();
    ping_init();

    while (1)
    {
        float dist_cm = ping_getDistance();

        if (dist_cm < 0)
        {
            lcd_printf("PING timeout\novf:%lu", g_overflow_count);
        }
        else
        {
            lcd_printf("ticks:%lu\nms:%.2f\ncm:%.1f\novf:%lu", g_last_pulse_ticks, g_last_pulse_ms, dist_cm, g_overflow_count);
        }

        timer_waitMillis(250);
    }
}
