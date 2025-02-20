//*****************************************************************************
//
// display.c - Displays helicopter statistics on the Orbit Board OLED
//
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
void displayStatistics(int alt_percent, int yaw_hund_deg, int alt_setpoint, int yaw_deg_setpoint, int tail_duty_cycle, int main_duty_cycle, heli_states_t heli_state)
{
    char string[17];  // 16 characters across the display

    // Display altitude
    usnprintf (string, sizeof(string), "Alt: %4d%% (%d%%)", alt_percent, alt_setpoint);
    OLEDStringDraw (string, 0, 0);

    // Display yaw
    int yaw_deg = sign(yaw_hund_deg) * abs(yaw_hund_deg) / 100;
    int yaw_dec_deg = (abs(yaw_hund_deg) % 100) / 10;
    usnprintf (string, sizeof(string), "Yaw:%4d.%01d (%03d)", yaw_deg, yaw_dec_deg, yaw_deg_setpoint);
    OLEDStringDraw (string, 0, 1);

    // Display duty cycles
    usnprintf (string, sizeof(string), "DC M:%2d%%, T:%2d%%", main_duty_cycle, tail_duty_cycle);
    OLEDStringDraw (string, 0, 2);

    // Display helicopter state
    char* state_string;
    switch (heli_state) {
        case LANDED:
            state_string = "LANDED        ";
            break;
        case CALIBRATING:
            state_string = "CALIBRATING   ";
            break;
        case TAKING_OFF:
            state_string = "TAKING_OFF    ";
            break;
        case LANDING:
            state_string = "LANDING       ";
            break;
        case FLYING:
            state_string = "FLYING        ";
            break;
        default:
            state_string = "ERROR         ";
            break;
    } 
    OLEDStringDraw (state_string, 0, 3);
    
    
}

