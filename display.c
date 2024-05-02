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


// Function to display the filtered ADC value (10-bit value, note) and sample count.
void displayStatistics(uint16_t filteredVal, uint16_t currentVal, int16_t heightPercent, uint32_t count, displayMode_t mode, int yaw, int yaw_hund_deg)
{
    char string[17];  // 16 characters across the display
    static displayMode_t lastMode = OFF;

    if (mode == OFF || mode != lastMode) {
        char emptyString[] = "                ";
        OLEDStringDraw (emptyString, 0, 0);
        OLEDStringDraw(emptyString, 0, 1);
        OLEDStringDraw (emptyString, 0, 2);
        OLEDStringDraw (emptyString, 0, 3);
        lastMode = mode;
    }

    if (mode == HEIGHT) {
        OLEDStringDraw ("Heli - Height", 0, 0);
        usnprintf (string, sizeof(string), "Height: %4d%%", heightPercent);
        OLEDStringDraw (string, 0, 1);
    } else if (mode == FILTERED) {
        OLEDStringDraw ("Heli - Filter", 0, 0);
        usnprintf (string, sizeof(string), "Filtered: %6d", filteredVal);
        OLEDStringDraw (string, 0, 1);
        usnprintf (string, sizeof(string), "Curr: %6d", currentVal);
        OLEDStringDraw (string, 0, 2);
    } else if (mode == YAW) {
        OLEDStringDraw ("Heli - Yaw", 0, 0);
        usnprintf (string, sizeof(string), "Yaw: %4d", yaw);
        OLEDStringDraw (string, 0, 1);
        int yaw_deg = yaw_hund_deg / 100;
        int yaw_dec_deg = yaw_hund_deg >= 0 ? yaw_hund_deg % 100 : (-yaw_hund_deg) % 100;
        usnprintf (string, sizeof(string), "Yaw: %4d.%02d", yaw_deg, yaw_dec_deg);
        OLEDStringDraw (string, 0, 2);
    }
}

