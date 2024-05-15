//*****************************************************************************
//
// yaw.c - gives  reliable continuous yaw monitoring with sub-degree precision.
//
//         The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
//         When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#ifndef YAW_H
#define YAW_H

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
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"


// CONSTANTS
#define DEGREES_PER_REV 360
#define INT_PINS GPIO_PIN_0 | GPIO_PIN_1
#define WHEEL_SLOTS 112
#define TRANSITIONS_PER_REV (WHEEL_SLOTS * 4)

/**Calls the interrupt handler for every rising or falling edge detected**/
void initYaw (void);


int getYawHundDeg(void);

int32_t getYawWrap(int32_t yaw_deg_abs, int32_t scale);

int getYawRaw(void);

int32_t getReferencePosition(int32_t yaw_hund_deg);

#endif
