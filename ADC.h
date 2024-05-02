/**  @file   ADC.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/

#ifndef ADC_H
#define ADC_H

// Constants
#define BUF_SIZE 70
#define SAMPLE_RATE_HZ 280
#define COEF_SCALE 10000
#define CUTOFF_FREQ 5.5 // Results in -3dB at 4Hz

#define PI 3.14159265358979323846
#define ADC_STEPS_PER_V (4096 * 10 / 33)

// Global variables
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static circBuf_t g_filteredBuffer;      // Buffer of size BUF_SIZE integers (filtered sample values)
static uint32_t g_ulSampCnt;    // Counter for the interrupts
int16_t g_coefs[BUF_SIZE];
int16_t g_heightPercent = 0;
int32_t g_zeroHeightValue = -1;

void initADC (void);

// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
void ADCIntHandler(void);

// The interrupt handler for the for SysTick interrupt.
void SysTickIntHandler(void);

void lpf_coefs(int16_t n, float f, int16_t fs, int16_t *coefs);


#endif





