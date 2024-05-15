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
#include "fsm.h"

#define ALT_MIN 0
#define ALT_MAX 100
#define YAW_STEP 15

#define SCHEDULE_FREQ 1000
#define PID_FREQ 200
#define BUTTONS_FREQ 10
#define DISPLAY_FREQ 10
#define UART_FREQ 200
#define FSM_FREQ 20

#define TIME_DELTA (10000 / PID_FREQ)

#define CALIBRATION_ROTATION_MARGIN 30

heli_states_t heli_state = LANDED;

uint64_t schedule_ticks = 0;

bool switch_active = false;

bool run_pid = false;
bool run_buttons = false;
bool run_display = false;
bool run_uart = false;
bool run_fsm = false;

void SysTickIntHandler(void)
{
    if (schedule_ticks % (1000 / SAMPLE_RATE_HZ) == 0) {
        triggerADC(); // preemptive
    }
    if (schedule_ticks % (1000 / PID_FREQ) == 0) {
        run_pid = true;
    }
    if (schedule_ticks % (1000 / DISPLAY_FREQ) == 0) {
        run_display = true;
    }
    if (schedule_ticks % (1000 / BUTTONS_FREQ) == 0) {
        run_buttons = true;
    }
    if (schedule_ticks % (1000 / UART_FREQ) == 0) {
        run_uart = true;
    }
    if (schedule_ticks % (1000 / FSM_FREQ) == 0) {
        run_fsm = true;
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
    int user_alt_setpoint = 50;
    int alt_setpoint = 0;
    int yaw_setpoint = 0;
    uint64_t utickCount = 0;

	while (1) {
	    if (run_pid) {
	        calculateControl(getHeightPercentage(), getYawHundDeg(), alt_setpoint * 100, yaw_setpoint * 100, TIME_DELTA);
	        run_pid = false;
	    } else if (run_buttons) {
            updateButtons();        // Checks for button press.

            if (checkButton(UP) == PUSHED) {
                user_alt_setpoint += ALT_STEP;
                user_alt_setpoint = user_alt_setpoint > ALT_MAX ? ALT_MAX : user_alt_setpoint;
            }

            if (checkButton(DOWN) == PUSHED) {
                user_alt_setpoint -= ALT_STEP;
                user_alt_setpoint = user_alt_setpoint < ALT_MIN ? ALT_MIN : user_alt_setpoint;
            }

            if (checkButton(LEFT) == PUSHED) {
               yaw_setpoint = getYawWrap(yaw_setpoint - YAW_STEP, 1);
           }

           if (checkButton(RIGHT) == PUSHED) {
               yaw_setpoint = getYawWrap(yaw_setpoint + YAW_STEP, 1);
           }

           if (checkButton(RIGHT_SW) == PUSHED) {
                if (heli_state == LANDED) {
                    heli_state = CALIBRATING;
                }
           } else if (checkButton(RIGHT_SW) == RELEASED) {
                if (heli_state == FLYING) {
                    heli_state = LANDING;
                }
           }

        //    if (checkButton(RESET) == PUSHED) {
        //         SysCtlReset
        //    }

           run_buttons = false;
        } else if (run_display) {
            displayStatistics(getHeightPercentage() / 100, getYawHundDeg(), alt_setpoint, yaw_setpoint, getTailDutyCycle(), getMainDutyCycle(), heli_state);
            run_display = false;
        } else if (run_uart) {
            helicopterInfo(getHeightPercentage() / 100, getYawHundDeg(), getTailDutyCycle(), getMainDutyCycle(), getControlTerms());
            run_uart = false;
        } else if (run_fsm) {
            switch (heli_state) {
                case LANDED:
                    alt_setpoint = 0;
                    yaw_setpoint = 0;
                    break;
                case CALIBRATING:
                    if (hasYawCalibrated()) {
                        heli_state = FLYING;
                        yaw_setpoint = getYawWrap(0, 1);
                    } else {
                        alt_setpoint = 5;
                        yaw_setpoint = getYawWrap(getYawHundDeg() / 100 + CALIBRATION_ROTATION_MARGIN, 1);
                    }
                    break;
                case FLYING:
                    alt_setpoint = user_alt_setpoint;
                    break;
            }

             run_fsm = false;
        }

        utickCount++;
	}

}

