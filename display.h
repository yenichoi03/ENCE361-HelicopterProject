//*****************************************************************************
//
// display.h - Header module for display.c
//
//
// Author:  ych227, sli219
//*****************************************************************************

#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "buttons4.h"
#include "fsm.h"

// Initialises the display
void initDisplay (void);

// Function to display key statistics of the helicopter
void displayStatistics(int alt_percent, int yaw_hund_deg, int alt_setpoint, int yaw_deg_setpoint, int tail_duty_cycle, int main_duty_cycle, heli_states_t heli_state);

#endif





