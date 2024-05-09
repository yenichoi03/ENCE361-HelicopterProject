//*****************************************************************************
//
// ADCdemo1.c - Simple interrupt driven program which samples with AIN0
//
// Author:  P.J. Bones	UCECE
// Last modified:	8.2.2018
//
//*****************************************************************************
// Based on the 'convert' series from 2016
//*****************************************************************************

#include <altitude.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>


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
#include "yaw.h"
#include "display.h"
#include "control.h"

// Initialisation functions for the clock (incl. SysTick), ADC, display
void initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}



int main(void)
{
	initClock ();
    initAltitude();
	initDisplay ();
    initButtons ();
    initYaw ();

    initControl();

    // Enable interrupts to the processor.
    IntMasterEnable();
    uint32_t utickCount = 0;


	while (1)
	{
	    if (utickCount > 300) {
	        calculateControl(getHeightPercentage(), getYawRaw(), 50, 90);
	    }

        updateButtons();

        if (checkButton(UP) == PUSHED) {
            // cycle through the display modes
            nextDisplayMode();
        }

        if (checkButton(LEFT) == PUSHED) {
            // reset the zero height value
            ZeroHeightReset();
        }

        if (utickCount % 3 == 0) {
            displayStatistics(getFilteredValue(), getCurrentValue(), getHeightPercentage(), getSampleCount(), getYawHundDeg(), getTailDutyCycle(), getMainDutyCycle());
        }

		SysCtlDelay (SysCtlClockGet() / 240);  // Update display at ~ 2 Hz
        utickCount++;
	}
}

