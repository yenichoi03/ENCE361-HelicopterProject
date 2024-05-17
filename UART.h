//*****************************************************************************
//
// UART.h - Header file for UART.C
//
//
// Author:  ych227, sli219
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
#include "control.h"
#include "fsm.h"

void initUART();

void helicopterInfo(int alt_percent, int yaw_hund_deg, int alt_setpoint, int yaw_setpoint, int tail_duty_cycle, int main_duty_cycle, control_terms_t control_terms, heli_states_t heli_state);

void UARTSend (char *pucBuffer);

#endif
