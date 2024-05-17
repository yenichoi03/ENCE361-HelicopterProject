//*****************************************************************************
//
// yaw.h - Header file for yaw.c
//
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
#define YAW_INT_PINS GPIO_PIN_0 | GPIO_PIN_1
#define WHEEL_SLOTS 112
#define TRANSITIONS_PER_REV (WHEEL_SLOTS * 4)

void initYaw (void);

bool hasYawCalibrated (void);

int getYawHundDeg(void);

int32_t getYawWrap(int32_t yaw_deg_abs, int32_t scale);

int32_t getReferencePosition(void);

#endif
