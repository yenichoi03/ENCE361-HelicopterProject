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

#include <altitude.h>

#define PI 3.14159265358979323846


// Global variables
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;    // Counter for the interrupts

int16_t g_heightPercent = 0;
int32_t g_zeroHeightValue = -1;
int32_t g_filteredValue = 0;

// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
static void ADCIntHandler(void)
{
    uint32_t ulValue;

    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
    // Place it in the circular buffer (advancing write index)
    writeCircBuf (&g_inBuffer, ulValue);

    int sum = 0;
    int j = 0;

    // move read pointer to the correct position
    setReadIndexToOldest(&g_inBuffer);

    for (j = 0; j < BUF_SIZE; j++) {
        sum += readCircBuf(&g_inBuffer, true);
    }
    sum /= BUF_SIZE;

    // calculate height value
    if (g_ulSampCnt > (BUF_SIZE * 4)) {
        if (g_zeroHeightValue == -1) {
            g_zeroHeightValue = sum;
        }
        g_heightPercent = (g_zeroHeightValue - sum) * 100 / ADC_STEPS_PER_V;
    }

    g_filteredValue = sum;

    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}

void ZeroHeightReset(void)
{
    g_zeroHeightValue = -1;
}

static void initADC(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE | ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE, 3);
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);
    ADCIntEnable(ADC0_BASE, 3);
}


void initAltitude (void)
{
    initCircBuf (&g_inBuffer, BUF_SIZE);
    initADC();
}


// The interrupt handler for the for SysTick interrupt.
void SysTickIntHandler(void)
{
    // Initiate a conversion
    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
}


uint32_t getCurrentValue(void)
{
    setReadIndexToNewest(&g_inBuffer);
    return readCircBuf(&g_inBuffer, false);
}


uint32_t getFilteredValue(void)
{
    return g_filteredValue;
}


int16_t getHeightPercentage(void)
{
    return g_heightPercent;
}


int16_t getSampleCount(void)
{
    return g_ulSampCnt;
}




