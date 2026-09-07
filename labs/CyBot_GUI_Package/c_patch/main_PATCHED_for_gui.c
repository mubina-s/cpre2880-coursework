#include "Timer.h"
#include "lcd.h"
#include "open_interface.h"
#include "movement.h"
#include "uart-interrupt.h"
#include "scan.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "driverlib/interrupt.h"

// IP address: 192.168.1.1    port: 288

extern int rescan_needed;

void sendSensorStatus(oi_t *sensor_data){
    char buffer[260];
    int bumperSeen;
    int cliffSeen;
    int boundarySeen;

    oi_update(sensor_data);

    bumperSeen = sensor_data->bumpLeft || sensor_data->bumpRight;
    cliffSeen = sensor_data->cliffLeft || sensor_data->cliffFrontLeft || sensor_data->cliffFrontRight || sensor_data->cliffRight || sensor_data->wheelDropLeft || sensor_data->wheelDropRight;
    boundarySeen = cliffSeen; //using the cliff sensor flags for the boundary/hole warning

    sprintf(buffer, "\r\nGUI_SENSOR,bumper=%d,cliff=%d,boundary=%d,bumpLeft=%d,bumpRight=%d,cliffLeft=%d,cliffFrontLeft=%d,cliffFrontRight=%d,cliffRight=%d,wheelDropLeft=%d,wheelDropRight=%d", bumperSeen, cliffSeen, boundarySeen, sensor_data->bumpLeft, sensor_data->bumpRight, sensor_data->cliffLeft, sensor_data->cliffFrontLeft, sensor_data->cliffFrontRight, sensor_data->cliffRight, sensor_data->wheelDropLeft, sensor_data->wheelDropRight);
    uart_sendStr(buffer);
}

//uart commands section
void handleManualCommand(char command, oi_t *sensor_data){
    char buffer[260];
    switch(command){
        //forward commands
        case '1':
            move_forward_manual(sensor_data, 100); //10cm
            break;
        case '2':
            move_forward_manual(sensor_data, 150); //15cm
            break;
        case '3':
            move_forward_manual(sensor_data, 250); //25cm
            break;
        case '4':
            move_forward_manual(sensor_data, 400); //40cm
            break;
        //backward commands
        case '5':
            move_backward_manual(sensor_data, 150); //15cm backward
            break;
        case '6':
            move_backward_manual(sensor_data, 200); //20cm backward
            break;
        //left turn commands
        case '7':
            turn_left_manual(sensor_data, 5); //5 degree left turn
            break;
        case '8':
            turn_left_manual(sensor_data, 10); //10 degree left turn
            break;
        case '9':
            turn_left_manual(sensor_data, 45); //45 degree left turn
            break;
        case '0':
            turn_left_manual(sensor_data, 90); //90 degree left turn
            break;
        //right turn commands
        case 'q':
            turn_right_manual(sensor_data, 5); //5 degree right turn
            break;
        case 'w':
            turn_right_manual(sensor_data, 10); //10 degree right turn
            break;
        case 'e':
            turn_right_manual(sensor_data, 45); //45 degree right turn
            break;
        case 'r':
            turn_right_manual(sensor_data, 90); //90 degree right turn
            break;
        //stop command
        case 't':
            oi_setWheels(0,0);
            uart_sendStr("\r\nEVENT:STOP");
            break;
        //sensor status command
        case 's':
            sendSensorStatus(sensor_data); //send sensor status through command not functional rn
            break;
        //scan commands
        case 'y':
            scan180();
            break;
        case 'u':
            scan360(sensor_data);
            break;
        case 'H':   //not supposed to be used during GUI
            //help command
            sprintf(buffer, "\r\nManual control commands:\r\n1-4: Move forward 10-40cm\r\n5-6: Move backward 10-20cm\r\n7-0: Turn left 5, 10, 45, 90 degrees\r\nq-r: Turn right 5-90 degrees\r\nt: Stop\r\ns: Sensor status\r\ny: 180 scan\r\nu: 360 scan");
            uart_sendStr(buffer);
            break;
    }
}

int main(void) {
    timer_init();
    lcd_init();
    uart_interrupt_init();
    scanInit();

    oi_t *sensor_data = oi_alloc();
    timer_waitMillis(1000);
    lcd_printf("Before oi init");
    timer_waitMillis(1000);
    oi_init(sensor_data);
    timer_waitMillis(1000);
    lcd_printf("After oi init");

    command_byte = 0;
    command_flag = 0;
    rescan_needed = 0;

    lcd_printf("Manual Mode");
    uart_sendStr("\r\nCYBOT MANUAL MODE READY\r\n");
    while(command_byte != 'S'){
        //busy wait until start
//        oi_update(sensor_data);
//        print_cliff_values(sensor_data);
//        timer_waitMillis(500);
    }

    while (1)
    {
        oi_update(sensor_data);

        if (emergency_detected(sensor_data))
        {
            sendSensorStatus(sensor_data);
            handle_emergency_stop(sensor_data);
            continue;
        }

        if (command_flag)
        {
            char cmd = command_byte;
            command_flag = 0;
            handleManualCommand(cmd, sensor_data);
        }
    }
}
