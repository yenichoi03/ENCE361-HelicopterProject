/*
 * UART.c
 *
 *  Created on: 6/05/2024
 *      Author: ych227
 */


#include "UART.h"

#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4
#define MAX_STR_LEN 16
//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE 9600
//#define UART_USB_BASE           UART0_BASE
//#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
//#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
//#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
//#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
//#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
//#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX


//Global Variable
char statusStr[MAX_STR_LEN + 1];
volatile uint8_t slowTick = false;


void initUSB_UART()
{
    // Enable GPIO port A which is used for UART0 pins.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Select the alternate (UART) function for these pins.
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    GPIOPinConfigure (GPIO_PA0_U0RX);       //RX
    GPIOPinConfigure (GPIO_PA1_U0TX);       //TX

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), BAUD_RATE, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);
    UARTFIFOEnable(UART0_BASE);
    UARTEnable(UART0_BASE);
}

void helicopterInfo(int alt_percent, int yaw_hund_deg, int tail_duty_cycle, int main_duty_cycle) {

    int yaw_deg = sign(yaw_hund_deg) * abs(yaw_hund_deg) / 100;
    int yaw_dec_deg = (abs(yaw_hund_deg) % 100) / 10;

    char string[50];
    usnprintf(string, sizeof(string), "Altitude: %4d%%    Yaw: %4d.%01d   Main Duty Cycle: %6d%%    Tail Duty Cycle: %6d%%", alt_percent, yaw_deg, yaw_dec_deg, main_duty_cycle, tail_duty_cycle);
    UARTSend(string);
}

void UARTSend (char *pucBuffer)
{
    // Loop while there are more characters to send.

    while(*pucBuffer)
    {
        // Write the next character to the UART Tx FIFO.
        UARTCharPut(UART0_BASE, *pucBuffer);
        pucBuffer++;

    }
}


