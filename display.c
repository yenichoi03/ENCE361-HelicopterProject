//*****************************************************************************
//
// display.c - Displays helicopter statistics on OLED board.
//
//             The helicopter altitude should be displayed as a percentage on the Orbit OLED board. 100 % should indicate maximum altitude, 0 % should
//             indicate minimum altitude (when the helicopter has landed).
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#include "display.h"


void initDisplay(void)
{
    OLEDInitialise ();      // intialise the Orbit OLED display
}

int sign(int num)
{
    return (num >= 0) ? 1 : -1;
}

// Function to display the filtered ADC value (10-bit value, note) and sample count.
void displayStatistics(int alt_percent, int yaw_hund_deg, int alt_setpoint, int yaw_deg_setpoint, int tail_duty_cycle, int main_duty_cycle)
{
    char string[17];  // 16 characters across the display

    // Display for altitude.
    usnprintf (string, sizeof(string), "Alt: %4d%% (%d%%)", alt_percent, alt_setpoint);
    OLEDStringDraw (string, 0, 0);

    // Display for yaw.
    int yaw_deg = sign(yaw_hund_deg) * abs(yaw_hund_deg) / 100;
    int yaw_dec_deg = (abs(yaw_hund_deg) % 100) / 10;
    usnprintf (string, sizeof(string), "Yaw:%4d.%01d (%03d)", yaw_deg, yaw_dec_deg, yaw_deg_setpoint);
    OLEDStringDraw (string, 0, 1);

    // Display for duty cucle for main motor
    usnprintf (string, sizeof(string), "Main: %6d%%", main_duty_cycle);
    OLEDStringDraw (string, 0, 2);

    // Display for duty cycle for tail motor
    usnprintf (string, sizeof(string), "Tail: %6d%%", tail_duty_cycle);
    OLEDStringDraw (string, 0, 3);
}

