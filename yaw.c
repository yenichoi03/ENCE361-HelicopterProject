/**  @file   ball.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/uart.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "yaw.h"

// GLOBAL VARIABLES
int32_t yaw = 0;
int32_t yaw_hund_deg = 0;
int32_t yaw_hund_deg_abs = 0;

int16_t QDE[4][4] = {{0, -1, 1, 0},
                     {1, 0, 0, -1},
                     {-1, 0, 0, 1},
                     {0, 1, -1, 0}};

// Quadrature encoder
static void yawIntHandler(void){
      static int prev_pin_state = 0;
      int pin_state = GPIOPinRead(GPIO_PORTB_BASE, INT_PINS);
      GPIOIntClear(GPIO_PORTB_BASE, INT_PINS);

      yaw += QDE[prev_pin_state][pin_state];
      prev_pin_state = pin_state;

      yaw_hund_deg_abs = yaw * 100 * DEGREES_PER_REV / TRANSITIONS_PER_REV;
      yaw_hund_deg = sign(yaw_hund_deg_abs) * ((abs(yaw_hund_deg_abs) + 18000 ) % 36000 - 18000);
}

int32_t getYawHundDeg(void)
{
    return yaw_hund_deg;
}

int32_t getYawHundDegAbs(void)
{
    return yaw_hund_deg_abs;
}

int32_t getYawRaw(void)
{
    return yaw;
}

void initYaw (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, INT_PINS);
    GPIOPadConfigSet(GPIO_PORTB_BASE, INT_PINS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTB_BASE, INT_PINS, GPIO_BOTH_EDGES);
    GPIOIntRegister(GPIO_PORTB_BASE, yawIntHandler);
    GPIOIntEnable(GPIO_PORTB_BASE, INT_PINS);
}

