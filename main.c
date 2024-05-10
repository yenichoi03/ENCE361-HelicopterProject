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
#include "UART.h"

#define ALT_MIN 0
#define ALT_MAX 100

#define PID_FREQ 480
#define TIME_DELTA (10000 / PID_FREQ)

#define WARMUP_SECONDS 4

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
    initialiseUSB_UART();

    int alt_setpoint = 50;
    int yaw_setpoint = 0; 
    int yaw_deg_setpoint = 0;

    // Enable interrupts to the processor.
    IntMasterEnable();
    uint64_t utickCount = 0;

	while (1) {
	    if (utickCount > PID_FREQ * WARMUP_SECONDS) {
	        calculateControl(getHeightPercentage(), getYawHundDegAbs(), alt_setpoint, yaw_setpoint, TIME_DELTA);
	    }

        if (utickCount % 5 == 0) {
            updateButtons();

            if (checkButton(UP) == PUSHED) {
                alt_setpoint += 10;
                alt_setpoint = alt_setpoint > ALT_MAX ? ALT_MAX : alt_setpoint;
            }

            if (checkButton(DOWN) == PUSHED) {
                alt_setpoint -= 10;
                alt_setpoint = alt_setpoint < ALT_MIN ? ALT_MIN : alt_setpoint;
            }

            if (checkButton(LEFT) == PUSHED) {
               yaw_setpoint += 15;
           }

           if (checkButton(RIGHT) == PUSHED) {
               yaw_setpoint -= 15;
           }
        }

        if (utickCount % 10 == 0) {
            displayStatistics(getHeightPercentage(), getYawHundDegAbs(), alt_setpoint, yaw_setpoint, getTailDutyCycle(), getMainDutyCycle());
        }

		SysCtlDelay (SysCtlClockGet() / PID_FREQ);  // Update display at ~ 2 Hz
        utickCount++;
	}

//	UARTSend("Yaw ");


}

