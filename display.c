/**  @file   ball.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"
#include "display.h"

void initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}

int sign(int num) {
    return (num >= 0) ? 1 : -1;
}

// Function to display the filtered ADC value (10-bit value, note) and sample count.
void displayStatistics(int alt_percent, int yaw_hund_deg, int alt_setpoint, int yaw_deg_setpoint, int tail_duty_cycle, int main_duty_cycle)
{
    char string[17];  // 16 characters across the display

    usnprintf (string, sizeof(string), "Alt: %4d%% (%d%%)", alt_percent, alt_setpoint);
    OLEDStringDraw (string, 0, 0);
    int yaw_deg = sign(yaw_hund_deg) * abs(yaw_hund_deg) / 100;
    int yaw_dec_deg = (abs(yaw_hund_deg) % 100) / 10;
    usnprintf (string, sizeof(string), "Yaw:%4d.%01d(%d)", yaw_deg, yaw_dec_deg, yaw_deg_setpoint);
    OLEDStringDraw (string, 0, 1);
    usnprintf (string, sizeof(string), "Main: %6d%%", main_duty_cycle);
    OLEDStringDraw (string, 0, 2);
    usnprintf (string, sizeof(string), "Tail: %6d%%", tail_duty_cycle);
    OLEDStringDraw (string, 0, 3);
}

