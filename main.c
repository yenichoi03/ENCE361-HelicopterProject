//*****************************************************************************
//
// ADCdemo1.c - Simple interrupt driven program which samples with AIN0
//
// Author:  P.J. Bones	UCECE
// Last modified:	8.2.2018
//
//*****************************************************************************
// Based on the 'convert' series from 2016
//*****************************************************************************

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

//---USB Serial comms: UART0, Rx:PA0 , Tx:PA1
#define BAUD_RATE 9600
#define UART_USB_BASE           UART0_BASE
#define UART_USB_PERIPH_UART    SYSCTL_PERIPH_UART0
#define UART_USB_PERIPH_GPIO    SYSCTL_PERIPH_GPIOA
#define UART_USB_GPIO_BASE      GPIO_PORTA_BASE
#define UART_USB_GPIO_PIN_RX    GPIO_PIN_0
#define UART_USB_GPIO_PIN_TX    GPIO_PIN_1
#define UART_USB_GPIO_PINS      UART_USB_GPIO_PIN_RX | UART_USB_GPIO_PIN_TX


#define PI 3.14159265358979323846
#define ADC_STEPS_PER_V (4096 * 10 / 33)

// Constants
#define BUF_SIZE 70
#define SAMPLE_RATE_HZ 280

// Global variables
static circBuf_t g_inBuffer;		// Buffer of size BUF_SIZE integers (sample values)
static circBuf_t g_filteredBuffer;		// Buffer of size BUF_SIZE integers (filtered sample values)
static uint32_t g_ulSampCnt;	// Counter for the interrupts
int16_t g_heightPercent = 0;
int32_t g_zeroHeightValue = -1;

typedef enum displayMode {HEIGHT = 0, FILTERED, OFF} displayMode_t;
#define DISPLAY_MODES 3

displayMode_t g_displayMode = HEIGHT;

// The interrupt handler for the for SysTick interrupt.
void SysTickIntHandler(void)
{
    // Initiate a conversion
    ADCProcessorTrigger(ADC0_BASE, 3); 
    g_ulSampCnt++;
}

// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
void ADCIntHandler(void)
{
	uint32_t ulValue;
	
	ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
	// Place it in the circular buffer (advancing write index)
	writeCircBuf (&g_inBuffer, ulValue);
    
    // filter the value and place it in the filtered circular buffer
    int sum = 0;
    int j = 0;
    // move read pointer to the correct position
    setReadIndexToOldest(&g_inBuffer);

    for (j = 0; j < BUF_SIZE; j++) {
        
        sum += readCircBuf(&g_inBuffer, true) * g_coefs[j];
        // sum += readCircBuf(&g_inBuffer, true);
    }
    // sum /= BUF_SIZE;

    // calculate height value
    if (g_ulSampCnt > (BUF_SIZE * 4)) {
        if (g_zeroHeightValue == -1) {
            g_zeroHeightValue = sum;
        }
        g_heightPercent = (g_zeroHeightValue - sum) * 100 / ADC_STEPS_PER_V;        
    }


    writeCircBuf (&g_filteredBuffer, sum);

	// Clean up, clearing the interrupt
	ADCIntClear(ADC0_BASE, 3);                          
}

// Initialisation functions for the clock (incl. SysTick), ADC, display
void initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}

void initADC (void)
{
    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    
    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);
  
    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                             ADC_CTL_END);    
                             
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);
  
    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);
  
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}

void initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}

// Function to display the filtered ADC value (10-bit value, note) and sample count.
void displayStatistics(uint16_t filteredVal, uint16_t currentVal, int16_t heightPercent, uint32_t count, displayMode_t mode)
{
	char string[17];  // 16 characters across the display
    static displayMode_t lastMode = OFF;

    if (mode == OFF || mode != lastMode) {
        char emptyString[] = "                ";
        OLEDStringDraw (emptyString, 0, 0);
        OLEDStringDraw(emptyString, 0, 1);
        OLEDStringDraw (emptyString, 0, 2);
        OLEDStringDraw (emptyString, 0, 3);
        lastMode = mode;
    }

    if (mode == HEIGHT) {
        OLEDStringDraw ("Heli - Height", 0, 0);
        usnprintf (string, sizeof(string), "Height (%%): %4d%%", heightPercent);
        OLEDStringDraw (string, 0, 1);
    } else if (mode == FILTERED) {
        OLEDStringDraw ("Heli - Filter", 0, 0);
        usnprintf (string, sizeof(string), "Filtered: %4d", filteredVal);
        OLEDStringDraw (string, 0, 1);
        usnprintf (string, sizeof(string), "Curr: %4d", currentVal);
        OLEDStringDraw (string, 0, 2);
    }
}

int main(void)
 {
	uint16_t i;
	int32_t sum;

    lpf_coefs(BUF_SIZE, 4, SAMPLE_RATE_HZ, g_coefs);
	
	initClock ();
	initADC ();
	initDisplay ();
	initCircBuf (&g_inBuffer, BUF_SIZE);
	initCircBuf (&g_filteredBuffer, BUF_SIZE);
    initButtons ();

    // Enable interrupts to the processor.
    IntMasterEnable();

    uint32_t utickCount = 0;

	while (1)
	{
        updateButtons();

        if (checkButton(UP) == PUSHED) {
            // cycle through the display modes
            g_displayMode = (g_displayMode + 1) % DISPLAY_MODES;
        }

        if (checkButton(LEFT) == PUSHED) {
            // reset the zero height value
            g_zeroHeightValue = -1;
        }

        if (utickCount % 3 == 0) {
            setReadIndexToNewest(&g_filteredBuffer);
            setReadIndexToNewest(&g_inBuffer);

            uint16_t currentVal = readCircBuf (&g_inBuffer, false);
            uint16_t filteredVal = readCircBuf (&g_filteredBuffer, false);

            displayStatistics(filteredVal, currentVal, g_heightPercent, g_ulSampCnt, g_displayMode);
        }

		SysCtlDelay (SysCtlClockGet() / 240);  // Update display at ~ 2 Hz
        utickCount++;
	}
}

