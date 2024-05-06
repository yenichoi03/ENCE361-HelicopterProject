/*
 * control.c
 *
 *  Created on: 6/05/2024
 *      Author: sli219
 */

#include "control.h"

#define PWM_PERIOD 100000

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

       //Fade
       bool fadeUp = true;
       unsigned long increment = 1;
       unsigned long pwmNow = 50;
       while(1)
       {
           SysCtlDelay( (SysCtlClockGet()/(3))*2 ) ;
           if (fadeUp) {
               pwmNow += increment;
               if (pwmNow >= 90) { fadeUp = false; }
           }
           else {
               pwmNow -= increment;
               if (pwmNow <= 10) { fadeUp = true; }
           }
           PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, PWM_PERIOD * pwmNow / 100);
           PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, PWM_PERIOD * pwmNow / 100);
       }
}




