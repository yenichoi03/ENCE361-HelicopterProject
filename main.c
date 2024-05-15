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

#define SCHEDULE_FREQ 1000
#define PID_FREQ 200
#define BUTTONS_FREQ 10
#define DISPLAY_FREQ 10
#define UART_FREQ 200

#define TIME_DELTA (10000 / PID_FREQ)

uint64_t schedule_ticks = 0;

bool run_pid = false;
bool run_buttons = false;
bool run_display = false;
bool run_uart = false;

void SysTickIntHandler(void)
{
    if (schedule_ticks % (1000 / SAMPLE_RATE_HZ) == 0) {
        triggerADC(); // preemptive
    }
    if (schedule_ticks % (1000 / PID_FREQ) == 1) {
        run_pid = true;
    }
    if (schedule_ticks % (1000 / DISPLAY_FREQ) == 2) {
        run_display = true;
    }
    if (schedule_ticks % (1000 / BUTTONS_FREQ) == 3) {
        run_buttons = true;
    }
    if (schedule_ticks % (1000 / UART_FREQ) == 4) {
        run_uart = true;
    }
    schedule_ticks++;
}

// Initialisation functions for the clock (incl. SysTick), ADC, display.
void initClock (void)
{
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);   // Set the clock rate to 20 MHz.
    SysTickPeriodSet(SysCtlClockGet() / SCHEDULE_FREQ);                                        // Set up the period for the SysTick timer.
    SysTickIntRegister(SysTickIntHandler);                                                      // Register the interrupt handler.
    SysTickIntEnable();                                                                         // Enable interrupt and device.
    SysTickEnable();
}


int main(void)
{
    // Initialisations.
	initClock();
    initAltitude();
	initDisplay();
    initButtons();
    initYaw();
    initControl();
    initUSB_UART();

    IntMasterEnable();
    int alt_setpoint = 50;
//    int yaw_setpoint = 10;
    int yaw_setpoint_wrap = 10;
    uint64_t utickCount = 0;

    bool reference_flag = false;
    
	while (1) {
	    if (run_pid) {
	        calculateControl(getHeightPercentage(), getYawHundDeg(), alt_setpoint * 100, yaw_setpoint_wrap * 100, TIME_DELTA);
	        run_pid = false;
	    } else if (run_buttons) {
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
               yaw_setpoint -= 15;
               yaw_setpoint_wrap = getYawWrap(yaw_setpoint, 1);
           }

           if (checkButton(RIGHT) == PUSHED) {
               yaw_setpoint += 15;
               yaw_setpoint_wrap = getYawWrap(yaw_setpoint, 1);
           }

           run_buttons = false;
        } else if (run_display) {
            displayStatistics(getHeightPercentage() / 100, getYawHundDeg(), alt_setpoint, yaw_setpoint_wrap, getTailDutyCycle(), getMainDutyCycle());
            run_display = false;
        } else if (run_uart) {
            helicopterInfo(getHeightPercentage() / 100, getYawHundDeg(), getTailDutyCycle(), getMainDutyCycle(), getControlTerms());
            run_uart = false;
        }


        if (utickCount % 200 == 1) {
            helicopterInfo(getHeightPercentage(), getYawHundDeg(), getTailDutyCycle(), getMainDutyCycle());

            if (yaw_setpoint != 0 && reference_flag == false) {
                yaw_setpoint = getReferencePosition(getYawHundDeg());
            } else if (getYawSetPoint() == 0) {
                reference_flag = true;
            }
        }


        if (utickCount % 70 == 1) {
            yaw_setpoint_wrap = getYawWrap(yaw_setpoint, 1);
        }
        utickCount++;
	}

}

