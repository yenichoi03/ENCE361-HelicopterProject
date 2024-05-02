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
#include "ADC.h"

#define BUF_SIZE 70

#define PI 3.14159265358979323846
#define ADC_STEPS_PER_V (4096 * 10 / 33)

// Global variables
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static circBuf_t g_filteredBuffer;      // Buffer of size BUF_SIZE integers (filtered sample values)
static uint32_t g_ulSampCnt;    // Counter for the interrupts
int16_t g_coefs[BUF_SIZE];
int16_t g_heightPercent = 0;
int32_t g_zeroHeightValue = -1;

// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
void ADCIntHandler(void)
{
    uint32_t ulValue;

    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
    // Place it in the circular buffer (advancing write index)
    writeCircBuf (&g_inBuffer, ulValue);

    // filter the value and place it in the filtered circular buffer
    int sum = 0;
    int j = 0;

    // move read pointer to the correct position
    setReadIndexToOldest(&g_inBuffer);

    for (j = 0; j < BUF_SIZE; j++) {
        sum += readCircBuf(&g_inBuffer, true) * g_coefs[j];
    }
    sum /= COEF_SCALE;

    // calculate height value
    if (g_ulSampCnt > (BUF_SIZE * 4)) {
        if (g_zeroHeightValue == -1) {
            g_zeroHeightValue = sum;
        }
        g_heightPercent = (g_zeroHeightValue - sum) * 100 / ADC_STEPS_PER_V;
    }


    writeCircBuf (&g_filteredBuffer, sum);

    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}

void ZeroHeightReset(void)
{
    g_zeroHeightValue = -1;
}

void initADC (void)
{
    lpf_coefs(BUF_SIZE, CUTOFF_FREQ, SAMPLE_RATE_HZ, g_coefs);

    initCircBuf (&g_inBuffer, BUF_SIZE);
    initCircBuf (&g_filteredBuffer, BUF_SIZE);
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                             ADC_CTL_END);

    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}



// The interrupt handler for the for SysTick interrupt.
void SysTickIntHandler(void)
{
    // Initiate a conversion
    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
}





uint32_t CurrentValue(void)
{
    setReadIndexToNewest(&g_inBuffer);
    return readCircBuf(&g_inBuffer, false);
}

uint32_t FilteredValue(void)
{
    setReadIndexToNewest(&g_filteredBuffer);
    return readCircBuf(&g_filteredBuffer, false);
}

int16_t HeightPercentageResult(void)
{
    return g_heightPercent;
}

int16_t SampleCountResult(void)
{
    return g_ulSampCnt;
}



// Chebyshev low-pass filter coefficients
void lpf_coefs(int16_t n, float f, int16_t fs, int16_t *coefs)
{
    int16_t i;
    float sum = 0;

    for (i = 0; i < n; i++) {
        if (i - n / 2 == 0) {
            coefs[i] = COEF_SCALE *  2 * f / fs;
        } else {
            coefs[i] = COEF_SCALE * (2 * PI * f * (i - n / 2) / fs) / (PI * (i - n / 2));
        }
        sum += coefs[i];
    }
    for (i = 0; i < n; i++) {
        coefs[i] = (COEF_SCALE * coefs[i]) / sum;
    }
}


