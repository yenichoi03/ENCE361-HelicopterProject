//*****************************************************************************
//
// main.c - Main code for helicopter project
//
// Author:  ych227, sli219
//
//
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
#define WARMUP_SECONDS 2

// Initialisation functions for the clock (incl. SysTick), ADC, display.
void initClock (void) {

    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);   // Set the clock rate to 20 MHz.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);                                        // Set up the period for the SysTick timer.
    SysTickIntRegister(SysTickIntHandler);                                                      // Register the interrupt handler.
    SysTickIntEnable();                                                                         // Enable interrupt and device.
    SysTickEnable();
}


int main(void) {

    // Initialisations.
	initClock();
    initAltitude();
	initDisplay();
    initButtons();
    initYaw();
    initControl();
    initUSB_UART();

    IntMasterEnable();              // Enable interrupts to the processor.
    int alt_setpoint = 50;
    int yaw_setpoint = 0;
    int yaw_setpoint_wrap = 0;
    uint64_t utickCount = 0;


	while (1) {
	    if (utickCount > PID_FREQ * WARMUP_SECONDS) {
	        calculateControl(getHeightPercentage(), getYawHundDeg(), alt_setpoint, yaw_setpoint_wrap * 100, TIME_DELTA);
	    }

        if (utickCount % 10 == 0) {
            updateButtons();        // Checks for button press.

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
               yaw_setpoint_wrap = getYawWrap(yaw_setpoint, 1);
           }

           if (checkButton(RIGHT) == PUSHED) {
               yaw_setpoint -= 15;
               yaw_setpoint_wrap = getYawWrap(yaw_setpoint, 1);
           }
        }
        
        if (utickCount % 20 == 0) {
            displayStatistics(getHeightPercentage(), getYawHundDeg(), alt_setpoint, yaw_setpoint_wrap, getTailDutyCycle(), getMainDutyCycle());
        }

        if (utickCount % 20 == 1) {
            helicopterInfo(getHeightPercentage(), getYawHundDeg(), getTailDutyCycle(), getMainDutyCycle());
        }


		SysCtlDelay(SysCtlClockGet() / PID_FREQ);  // Update display at ~ 2 Hz
        utickCount++;
	}

}

