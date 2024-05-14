//*****************************************************************************
//
// altitude.c - Calculates altitude from ADC
//
//         The samples (ADC outputs) are stored continually in a circular buffer. At regular intervals the mean value of the samples in the buffer are computed.
//
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#ifndef ADC_H
#define ADC_H

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

// Constants

#define SAMPLE_RATE_HZ 64
#define BUF_SIZE 8
#define COEF_SCALE 10000
#define ADC_STEPS_PER_V (4096 * 10 / 33)

void initAltitude (void);

// The interrupt handler for the for SysTick interrupt.
void SysTickIntHandler(void);

uint32_t getFilteredValue(void);

uint32_t getCurrentValue(void);

int16_t getHeightPercentage(void);

int16_t getSampleCount(void);

void ZeroHeightReset(void);


#endif





