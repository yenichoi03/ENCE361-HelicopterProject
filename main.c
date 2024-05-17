//*****************************************************************************
//
// main.c - Main code for helicopter project
//
// Author:  ych227, sli219
//*****************************************************************************

// #define DEBUG

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
#define ALT_STEP 10

#define SCHEDULE_FREQ 2000
#define PID_FREQ 40
#define BUTTONS_FREQ 60
#define DISPLAY_FREQ 10
#define UART_FREQ 700 // characters per second
#define FSM_FREQ 10

#define TIME_DELTA (10000 / PID_FREQ)

// how far ahead the setpoint should be during yaw calibration
#define CALIBRATION_ROTATION_MARGIN 30 

heli_states_t heli_state = LANDED;

uint64_t schedule_ticks = 0;

bool run_pid = false;
bool run_buttons = false;
bool run_display = false;
bool run_uart = false;
bool run_fsm = false;

// Scheduler Interrupt
void SysTickIntHandler(void)
{
    if (schedule_ticks % (SCHEDULE_FREQ / SAMPLE_RATE_HZ) == 0) {
        triggerADC(); // preemptive
    }
    if (schedule_ticks % (SCHEDULE_FREQ / PID_FREQ) == 0) {
        run_pid = true;
    }
    if (schedule_ticks % (SCHEDULE_FREQ / DISPLAY_FREQ) == 0) {
        run_display = true;
    }
    if (schedule_ticks % (SCHEDULE_FREQ / BUTTONS_FREQ) == 0) {
        run_buttons = true;
    }
    if (schedule_ticks % (SCHEDULE_FREQ / UART_FREQ) == 0) {
        run_uart = true;
    }
    if (schedule_ticks % (SCHEDULE_FREQ / FSM_FREQ) == 0) {
        run_fsm = true;
    }
    schedule_ticks++;
}

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
    initUART();
    IntMasterEnable();

    bool sw_has_been_low = false;
    bool reset_has_been_off = false;

    int alt_setpoint = 0;
    int yaw_setpoint = 0;

    int max_main_duty;
    int max_tail_duty;

	while (1) {
	    if (run_pid) {
	        calculateControl(getHeightPercentage(), getYawHundDeg(), alt_setpoint * 100, yaw_setpoint * 100, TIME_DELTA, max_main_duty, max_tail_duty);
	        run_pid = false;
	    } else if (run_buttons) {
            updateButtons();

            if (heli_state == FLYING) {
                if (checkButton(UP) == PUSHED) {
                    alt_setpoint += ALT_STEP;
                    alt_setpoint = alt_setpoint > ALT_MAX ? ALT_MAX : alt_setpoint;
                }

                if (checkButton(DOWN) == PUSHED) {
                    alt_setpoint -= ALT_STEP;
                    alt_setpoint = alt_setpoint < ALT_MIN ? ALT_MIN : alt_setpoint;
                }

                if (checkButton(LEFT) == PUSHED) {
                   yaw_setpoint = getYawWrap(yaw_setpoint - YAW_STEP, 1);
               }

               if (checkButton(RIGHT) == PUSHED) {
                   yaw_setpoint = getYawWrap(yaw_setpoint + YAW_STEP, 1);
               }
           }


           uint8_t right_sw_state = checkButton(RIGHT_SW);
           if (right_sw_state == PUSHED && sw_has_been_low) {
                if (heli_state == LANDED) {
                    heli_state = TAKING_OFF;
                }
           }

           if (right_sw_state == RELEASED) {
                sw_has_been_low = true;
                if (heli_state == FLYING) {
                    heli_state = LANDING;
                }
           }

            if (checkButton(RESET) == PUSHED) {
                 SysCtlReset();
            }

           run_buttons = false;
        } else if (run_display) {
            displayStatistics(getHeightPercentage() / 100, getYawHundDeg(), alt_setpoint, yaw_setpoint, getTailDutyCycle(), getMainDutyCycle(), heli_state);
            run_display = false;
        } else if (run_uart) {
            helicopterInfo(getHeightPercentage() / 100, getYawHundDeg(), alt_setpoint, yaw_setpoint, getTailDutyCycle(), getMainDutyCycle(), getControlTerms(), heli_state);
            run_uart = false;
        } else if (run_fsm) {
            switch (heli_state) {
                case LANDED:
                    alt_setpoint = 0;
                    yaw_setpoint = 0;
                    max_main_duty = 0;
                    max_tail_duty = 0;
                    break;
                case TAKING_OFF:
                    alt_setpoint = 5;
                    max_tail_duty = DUTY_CYCLE_MAX;
                    if (getHeightPercentage() / 100 > 3) {
                        heli_state = CALIBRATING;
                    } else if (max_main_duty < DUTY_CYCLE_MAX) {
                        max_main_duty += 1;
                    }
                    break;
                case CALIBRATING:
                    max_main_duty = DUTY_CYCLE_MAX;
                    max_tail_duty = DUTY_CYCLE_MAX;
                    if (hasYawCalibrated()) {
                        alt_setpoint = 50;
                        heli_state = FLYING;
                        yaw_setpoint = getYawWrap(0, 1);
                    } else {
                        alt_setpoint = 5;
                        yaw_setpoint = getYawWrap(getYawHundDeg() / 100 + CALIBRATION_ROTATION_MARGIN, 1);
                    }
                    break;
                case FLYING:
                    max_main_duty = DUTY_CYCLE_MAX;
                    max_tail_duty = DUTY_CYCLE_MAX;
                    break;
                case LANDING:
                    yaw_setpoint = 0;
                    if (abs(getYawHundDeg() / 100) < 4) {
                        if (alt_setpoint > 0) {
                            alt_setpoint -= 1;
                        }
                        if (getHeightPercentage() / 100 < 3) {
                            if (max_main_duty > 0) {
                                max_main_duty -= 10;
                            } else {
                                max_tail_duty = 0;
                                max_main_duty = 0;
                                heli_state = LANDED;
                            }
                        }
                    }
                    break;
            }

             run_fsm = false;
        }
	}
}

