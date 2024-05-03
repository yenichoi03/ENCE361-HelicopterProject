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

// Quadrature encoder
static void yawIntHandler(void){

    int pin_state = GPIOIntStatus(GPIO_PORTB_BASE, true);
    int prev_pin_state = GPIOPinRead(GPIO_PORTB_BASE, INT_PINS);
    GPIOIntClear(GPIO_PORTB_BASE, INT_PINS);
    static int last_pin = -1;
    static int last_transition = -1;

    int current_pin = 0;
    if (pin_state == 2) {
        current_pin = 1;
    } else if (pin_state == 3) {
        last_pin = -1;
        last_transition = -1;
        return;
    }

    int current_transition = 0;

    if (current_pin == 0) {
        current_transition = rising_edge & 1;
    } else {
        current_transition = (rising_edge & 2) >> 1;
    }

    if (last_pin == -1) {
        last_pin = current_pin;
        last_transition = current_transition;
        return;
    }

    if (last_pin != current_pin) {
        if (current_pin == 1) {
            if (last_transition == current_transition) {
                yaw--;
            } else {
                yaw++;
            }
        } else {
            if (last_transition != current_transition) {
                yaw--;
            } else {
                yaw++;
            }
        }
    }

    // TODO: Add more accurate reverse yaw tracking where last_pin == current_pin

    yaw_hund_deg = yaw * 100 * DEGREES_PER_REV / TRANSITIONS_PER_REV;

    last_pin = current_pin;
    last_transition = current_transition;

}

int32_t GetYawHundDeg(void)
{
    return yaw_hund_deg;
}

void initYaw (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, INT_PINS);
    GPIOPadConfigSet(GPIO_PORTB_BASE, INT_PINS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTB_BASE, INT_PINS, GPIO_BOTH_EDGES);
    GPIOIntRegister(GPIO_PORTB_BASE, YawIntHandler);
    GPIOIntEnable(GPIO_PORTB_BASE, INT_PINS);
}

