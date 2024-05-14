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


#include "UART.h"

#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4
#define MAX_STR_LEN 16
#define BAUD_RATE 9600

// GLOBAL VARIABLES
char statusStr[MAX_STR_LEN + 1];
volatile uint8_t slowTick = false;


// Initialisations for UART communications from port A and UART0 pins
void initUSB_UART() {

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);                    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);      // Select the alternate (UART) function for these pins.
    GPIOPinConfigure (GPIO_PA0_U0RX);                               //RX
    GPIOPinConfigure (GPIO_PA1_U0TX);                               //TX

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), BAUD_RATE, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART0_BASE);
    UARTEnable(UART0_BASE);
}


// Compiles helicopter altitude, yaw, main motor duty cycle and tail motor duty cycle and sends informations through UART transmission.
void helicopterInfo(int alt_percent, int yaw_hund_deg, int tail_duty_cycle, int main_duty_cycle) {

    int yaw_deg = sign(yaw_hund_deg) * abs(yaw_hund_deg) / 100;
    int yaw_dec_deg = (abs(yaw_hund_deg) % 100) / 10;

    char string[150];
    usnprintf(string, sizeof(string), "Altitude: %2d%% Yaw: %2d.%01d Main Duty Cycle: %3d%% Tail Duty Cycle: %3d%%\n", alt_percent, yaw_deg, yaw_dec_deg, main_duty_cycle, tail_duty_cycle);
    UARTSend(string);
}


void UARTSend(char *pucBuffer) {

    while(*pucBuffer) {
        UARTCharPut(UART0_BASE, *pucBuffer);        // Write the next character to the UART Tx FIFO.
        pucBuffer++;
    }
}


