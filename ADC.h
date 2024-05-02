/**  @file   ADC.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/

#ifndef ADC_H
#define ADC_H

void initADC (void);

// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
void ADCIntHandler(void);

// The interrupt handler for the for SysTick interrupt.
void SysTickIntHandler(void);

void lpf_coefs(int16_t n, float f, int16_t fs, int16_t *coefs);


#endif





