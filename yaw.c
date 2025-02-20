//*****************************************************************************
//
// yaw.c - gives  reliable continuous yaw monitoring with sub-degree precision using interrupts
//
// Author:  ych227, sli219
//*****************************************************************************

#include "yaw.h"


// GLOBAL VARIABLES
int32_t yaw = 0;
int32_t yaw_hund_deg = 0;
int32_t yaw_hund_deg_abs = 0;
bool has_calibrated = false;

int16_t QDE[4][4] = {{0, -1, 1, 0},
                     {1, 0, 0, -1},
                     {-1, 0, 0, 1},
                     {0, 1, -1, 0}};


static void referenceIntHandler(void)
{
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
    yaw = 0;
    has_calibrated = true;
}

// Quadrature encoder interrupt
static void yawIntHandler(void)
{
      static int prev_pin_state = 0;
      int8_t pin_state = GPIOPinRead(GPIO_PORTB_BASE, YAW_INT_PINS);

      GPIOIntClear(GPIO_PORTB_BASE, YAW_INT_PINS);

      yaw += QDE[prev_pin_state][pin_state];
      prev_pin_state = pin_state;

      yaw_hund_deg_abs = yaw * 100 * DEGREES_PER_REV / TRANSITIONS_PER_REV;
      yaw_hund_deg = getYawWrap(yaw_hund_deg_abs, 100);
}

bool hasYawCalibrated(void) {
    return has_calibrated;
}

int32_t getYawHundDeg(void)
{
    return yaw_hund_deg;
}

int32_t getYawWrap(int32_t yaw_deg_abs, int32_t scale)
{
    return sign(yaw_deg_abs) * ((abs(yaw_deg_abs) + (180 * scale)) % (360 * scale) - (180 * scale));
}

// Initialises GPIO ports and pins to read yaw
void initYaw (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, YAW_INT_PINS);
    GPIOPadConfigSet(GPIO_PORTB_BASE, YAW_INT_PINS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTB_BASE, YAW_INT_PINS, GPIO_BOTH_EDGES);
    GPIOIntRegister(GPIO_PORTB_BASE, yawIntHandler);
    GPIOIntEnable(GPIO_PORTB_BASE, YAW_INT_PINS);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOIntRegister(GPIO_PORTC_BASE, referenceIntHandler);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4);

}

