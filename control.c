//*****************************************************************************
//
// contorl.c - Sets the PWM for the main and tail motors using PID control
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#include "control.h"

#define PWM_PERIOD 100000
#define PWM_MAIN 0
#define PWM_TAIL 1
#define SCALE 1000
#define MAX_REASONABLE_STEADY_STATE_ERROR 25

#define TAIL_I_MAX 3

// Controller coefficients for main motor
const int Kp_main = 20;
const int Ki_main = 3;
const int Kd_main = 40;
const int gravity_offset_pc = 33;

// Controller coefficients for tail motor
const int Kp_tail = 120;
const int Ki_tail = 40;
const int Kd_tail = 80;
const int tail_coupling_pc = 80;

int main_duty_cycle;
int tail_duty_cycle;

control_terms_t tail_terms = {0};
control_terms_t main_terms = {0};

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
      SysCtlPWMClockSet(SYSCTL_PWMDIV_1);               // Sets the clock and configure PWM Clock to match system


       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);     // Enable the peripherals used by this program.
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);      // The Tiva Launchpad has two modules (0 and 1). Module 1 covers the LED pins
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);

       //Configure PF1,PF2,PF3 Pins as PWM
       GPIOPinConfigure(GPIO_PC5_M0PWM7);
       GPIOPinConfigure(GPIO_PF1_M1PWM5);
       GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_5);
       GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1);

       //Configure PWM Options
       PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);      //PWM_GEN_3 Covers M1PWM6 and M1PWM7 See page 207 4/11/13 DriverLib doc
       PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);      //PWM_GEN_2 Covers M1PWM4 and M1PWM5

       //Set the Period (expressed in clock ticks)
       PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, PWM_PERIOD);
       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, PWM_PERIOD);

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


int getTailDutyCycle(void)
{
    return tail_duty_cycle;
}

control_terms_t getControlTerms(void)
{
    return tail_terms;
}

static int32_t getYawWrap(int32_t yaw_deg_abs, int32_t scale)
{
    return sign(yaw_deg_abs) * ((abs(yaw_deg_abs) + (180 * scale)) % (360 * scale) - (180 * scale));
}

static int mainController (int altitude, int altitude_setpoint, int time_delta, int max_main_duty)
{
     static int prev_altitude = 0;

     main_terms.error = SCALE * altitude_setpoint - SCALE * altitude;
     main_terms.P = Kp_main * main_terms.error;
     int main_dI = Ki_main * main_terms.error * time_delta / 10000;
     main_terms.D = Kd_main * (int64_t)(prev_altitude * SCALE - altitude * SCALE) * 1000 / time_delta;


     int control = (main_terms.P + (main_terms.I + main_dI) + main_terms.D ) / (SCALE * SCALE) + gravity_offset_pc;

     if (max_main_duty > DUTY_CYCLE_MAX) {
        max_main_duty = DUTY_CYCLE_MAX;
    }

     int min_main_duty = max_main_duty < DUTY_CYCLE_MIN ? max_main_duty : DUTY_CYCLE_MIN;

     if (control > max_main_duty) {
         control = max_main_duty;
     } else if (control < min_main_duty) {
         control = min_main_duty;
     } else {
         main_terms.I = (main_terms.I + main_dI);
     }

     prev_altitude = altitude;

     return control;
}


static int tailController (int yaw_hund_deg, int yaw_hund_deg_setpoint, int main_output, int time_delta, int max_tail_duty)
{

    static int prev_yaw = 0;

    tail_terms.error = yaw_hund_deg_setpoint * SCALE - yaw_hund_deg * SCALE;
    tail_terms.error = getYawWrap(tail_terms.error, 100 * SCALE);

    tail_terms.P = Kp_tail * tail_terms.error / 10;
    int64_t tail_dI = Ki_tail * tail_terms.error * time_delta / 100000;
    int64_t tail_dError = getYawWrap(prev_yaw * SCALE - yaw_hund_deg * SCALE, 100 * SCALE);
    tail_terms.D = Kd_tail * (tail_dError) * 1000 / time_delta;

    if (tail_terms.P / (SCALE * SCALE) > DUTY_CYCLE_MAX) {
        tail_terms.P = DUTY_CYCLE_MAX * (SCALE * SCALE);
    }

    int control = (tail_terms.P + tail_terms.I + tail_dI + tail_terms.D) / (SCALE * SCALE) + ((tail_coupling_pc * main_output) / 100);

    if (max_tail_duty > DUTY_CYCLE_MAX) {
        max_tail_duty = DUTY_CYCLE_MAX;
    }

    int min_tail_duty = max_tail_duty < DUTY_CYCLE_MIN ? max_tail_duty : DUTY_CYCLE_MIN;

    if (control > max_tail_duty) {
        control = max_tail_duty;
    } else if (control < min_tail_duty) {
        control = min_tail_duty;
    } else if (abs(tail_terms.error / (SCALE * 100)) < MAX_REASONABLE_STEADY_STATE_ERROR) {
        tail_terms.I = (tail_terms.I + tail_dI);
    }

    prev_yaw = yaw_hund_deg;

    return control;
}


void calculateControl(int altitude, int yaw, int altitude_setpoint, int yaw_setpoint, int time_delta, int max_main_duty, int max_tail_duty)
{
   main_duty_cycle = mainController(altitude, altitude_setpoint, time_delta, max_main_duty);
   tail_duty_cycle = tailController(yaw, yaw_setpoint, main_duty_cycle, time_delta, max_tail_duty);
   setPWM(main_duty_cycle, PWM_MAIN);
   setPWM(tail_duty_cycle, PWM_TAIL);
}

