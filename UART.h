//*****************************************************************************
//
// UART.c - Code for UART communications
//
//          Information on the status of the helicopter should be transmitted via a serial link from UART0 at 9600 baud, with 1 stop bit and no parity bit
//          in each transmitted byte. Status information should include the desired and actual yaw (in degrees), the desired and actual altitude (as a
//          percentage of the maximum altitude), the duty cycle of each of the PWM signals controlling the rotors (%, with 0 meaning off) Updates should be
//          transmitted at regular intervals (no fewer than 4 updates per second).
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#ifndef UART_H
#define UART_H

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
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "circBufT.h"
#include "yaw.h"

void initUSB_UART();

void helicopterInfo(int alt_percent, int yaw_hund_deg, int tail_duty_cycle, int main_duty_cycle);

void UARTSend (char *pucBuffer);

#endif
