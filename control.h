//*****************************************************************************
//
// contorl.c - Sets the PWM and PID control for helicopter
//
//             The frequency of the PWM signals should be in the range from 150 Hz to 300 Hz.
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#ifndef CONTROL_H
#define CONTROL_H

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

typedef struct {
    int P;
    int I;
    int D;
    int error;
} control_terms_t;

void initControl(void);

void calculateControl(int altitude, int yaw, int altitude_setpoint, int yaw_setpoint, int time_delta);

control_terms_t getControlTerms(void);

int getTailDutyCycle();

int getTailError();

int getMainDutyCycle();

#endif
