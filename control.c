/*
 * control.c
 *
 *  Created on: 6/05/2024
 *      Author: sli219
 */

#include "control.h"

#define PWM_PERIOD 100000

#define DUTY_CYCLE_MAX 70
#define DUTY_CYCLE_MIN 10

#define PWM_MAIN 0
#define PWM_TAIL 1

#define SCALE 1000

// MAIN
const int Kp_main = 16500;
const int Ki_main = 4500;
const int Kd_main = 0;
const int gravity_offset_pc = 33;

const int Kp_tail = 16000;
const int Ki_tail = 0;
const int Kd_tail = 0;
const int tail_coupling_pc = 80;

int main_duty_cycle;
int tail_duty_cycle;

static void setPWM(int duty_cycle, int PWM)
{
    if (PWM == PWM_MAIN) {
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, PWM_PERIOD * duty_cycle / 100);
    } else if (PWM == PWM_TAIL) {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, PWM_PERIOD * duty_cycle / 100);
    }
}

void initControl(void)
{
    //Set the clock
      //Configure PWM Clock to match system
      SysCtlPWMClockSet(SYSCTL_PWMDIV_1);

      // Enable the peripherals used by this program.
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  //The Tiva Launchpad has two modules (0 and 1). Module 1 covers the LED pins
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

       //Configure PF1,PF2,PF3 Pins as PWM
       GPIOPinConfigure(GPIO_PC5_M0PWM7);
       GPIOPinConfigure(GPIO_PF1_M1PWM5);
       GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_5);
       GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

       //Configure PWM Options
       //PWM_GEN_2 Covers M1PWM4 and M1PWM5
       //PWM_GEN_3 Covers M1PWM6 and M1PWM7 See page 207 4/11/13 DriverLib doc
       PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
       PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

       //Set the Period (expressed in clock ticks)
       PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PWM_PERIOD);
       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, PWM_PERIOD);

       //Set PWM duty-50% (Period /2)
       PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, PWM_PERIOD / 2);
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, PWM_PERIOD / 2);

       // Enable the PWM generator
       PWMGenEnable(PWM0_BASE, PWM_GEN_3);
       PWMGenEnable(PWM1_BASE, PWM_GEN_2);

       // Turn on the Output pins
       PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, true);
       PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true);

       setPWM(0, PWM_MAIN);
       setPWM(0, PWM_TAIL);
}

int getMainDutyCycle()
{
    return main_duty_cycle;
}

int getTailDutyCycle()
{
    return tail_duty_cycle;
}


static int clampDutyCycle(int duty_cycle)
{
    if (duty_cycle > DUTY_CYCLE_MAX) {
        return DUTY_CYCLE_MAX;
    } else if (duty_cycle < DUTY_CYCLE_MIN) {
        return DUTY_CYCLE_MIN;
    }
    return duty_cycle;
}


static int mainController (int altitude, int altitude_setpoint, int time_delta)
{
     static int I = 0;
     static int prev_error = 0;
     int error = SCALE * altitude_setpoint - SCALE * altitude;
     int P = Kp_main * error;
     int dI = Ki_main * error * time_delta / 10000;
     int D = Kd_main * (error - prev_error) * 10000 / time_delta;

     int control = (P + (I + dI) + D ) / (SCALE * SCALE) + gravity_offset_pc;
     if (control < DUTY_CYCLE_MAX && control > DUTY_CYCLE_MIN) {
         I = (I + dI);
     }
     prev_error = error;

     int duty_cycle = clampDutyCycle(control);

     return duty_cycle;
}

static int tailController (int yaw_hund_deg, int yaw_hund_deg_setpoint, int main_output, int time_delta)
{
    static int I = 0;
    static int prev_error = 0;
    int tail_error = yaw_hund_deg_setpoint * SCALE - yaw_hund_deg * SCALE;
    tail_error = tail_error > 18000 * SCALE ? 36000 * SCALE - tail_error : tail_error;
    int P = Kp_tail * tail_error;
    int dI = Ki_tail * tail_error * time_delta / 10000;
    int D = Kd_tail * (tail_error - prev_error) * 10000 / time_delta ;

    int control = (P + (I + dI) + D) / (SCALE * SCALE) + ((tail_coupling_pc * main_output) / 100);
    if (control < DUTY_CYCLE_MAX && control > DUTY_CYCLE_MIN) {
        I = (I + dI);
    }

    prev_error = tail_error;

    int tail_duty_cycle = clampDutyCycle(control);

    return tail_duty_cycle;
}


void calculateControl(int altitude, int yaw, int altitude_setpoint, int yaw_setpoint, int time_delta)
{
   main_duty_cycle = mainController(altitude, altitude_setpoint, time_delta);
   tail_duty_cycle = tailController(yaw, yaw_setpoint, main_duty_cycle, time_delta);
   setPWM(main_duty_cycle, PWM_MAIN);
   setPWM(tail_duty_cycle, PWM_TAIL);
}

