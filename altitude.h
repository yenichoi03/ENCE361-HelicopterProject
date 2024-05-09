/**  @file   ADC.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/

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





