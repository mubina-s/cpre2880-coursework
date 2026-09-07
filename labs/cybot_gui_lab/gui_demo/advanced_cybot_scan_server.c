#include <stdio.h>
#include <string.h>
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"
#include "cyBot_Scan.h"

#define COMMAND_BUFFER_SIZE 64
#define SCAN_STEP_DEGREES   2

static void send_string(const char *text)
{
    while (*text != '\0')
    {
        cyBot_sendByte(*text);
        text++;
    }
}

static void get_command(char *command, int max_len)
{
    int index = 0;
    char ch = cyBot_getByte();

    while ((ch != '\n') && (index < max_len - 1))
    {
        if (ch != '\r')
        {
            command[index] = ch;
            index++;
        }
        ch = cyBot_getByte();
    }

    command[index] = '\0';
}

static void send_status(void)
{
    char buffer[80];
    sprintf(buffer, "CYBOT_READY version=%u\n", cyBOT_scan_version());
    send_string(buffer);
}

static void send_scan_data(void)
{
    cyBOT_Scan_t scan;
    char buffer[80];
    int angle;
    float meters;

    send_string("Angle(Degrees)\tDistance(m)\n");

    for (angle = 0; angle <= 180; angle += SCAN_STEP_DEGREES)
    {
        cyBOT_Scan(angle, &scan);

        // sound_dist is in centimeters, convert to meters for the GUI/file.
        meters = scan.sound_dist / 100.0f;

        sprintf(buffer, "%d\t%.3f\n", angle, meters);
        send_string(buffer);
    }

    send_string("END\n");
}

int main(void)
{
    char command[COMMAND_BUFFER_SIZE];

    timer_init();
    lcd_init();
    cyBot_uart_init();

 	right_calibration_value = 238000;
	left_calibration_value = 1188250;

    // Enable servo + ping + IR.
    cyBOT_init_Scan(0b0111);

    lcd_printf("GUI server ready");

    while (1)
    {
        get_command(command, COMMAND_BUFFER_SIZE);

        if (strlen(command) == 0)
        {
            continue;
        }

        lcd_printf("Cmd: %s", command);

        if ((strcmp(command, "H") == 0) || (strcmp(command, "HELLO") == 0))
        {
            send_string("HELLO_FROM_CYBOT\n");
        }
        else if ((strcmp(command, "S") == 0) || (strcmp(command, "STATUS") == 0))
        {
            send_status();
        }
        else if ((strcmp(command, "M") == 0) || (strcmp(command, "SCAN") == 0) ||
                 (strcmp(command, "m") == 0) || (strcmp(command, "scan") == 0))
        {
            lcd_printf("Scanning...");
            send_scan_data();
            lcd_printf("Scan done");
        }
        else if ((strcmp(command, "quit") == 0) || (strcmp(command, "Q") == 0))
        {
            send_string("BYE\n");
        }
        else
        {
            send_string("UNKNOWN_COMMAND\n");
        }
    }

    return 0;
}
