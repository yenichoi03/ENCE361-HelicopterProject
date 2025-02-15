//*****************************************************************************
//
// control.h - Defines types for control.c
//
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
    int64_t P;
    int64_t I;
    int64_t D;
    int64_t error;
} control_terms_t;

#define DUTY_CYCLE_MAX 70
#define DUTY_CYCLE_MIN 10

void initControl(void);

void calculateControl(int altitude, int yaw, int altitude_setpoint, int yaw_setpoint, int time_delta, int max_main_duty, int max_tail_duty);

control_terms_t getControlTerms(void);

int getTailDutyCycle();

int getTailError();

int getMainDutyCycle();

#endif
