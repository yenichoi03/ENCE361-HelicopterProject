//*****************************************************************************
//
// yaw.c - gives  reliable continuous yaw monitoring with sub-degree precision.
//
//         The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
//         When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#include "yaw.h"


// GLOBAL VARIABLES
int32_t yaw = 0;
int32_t yaw_hund_deg = 0;
int32_t yaw_hund_deg_abs = 0;

int16_t QDE[4][4] = {{0, -1, 1, 0},
                     {1, 0, 0, -1},
                     {-1, 0, 0, 1},
                     {0, 1, -1, 0}};


static void referenceIntHandler(void)
{
    int8_t pin_reference_state = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4);

    getReferencePosition(yaw_hund_deg);
    GPIOIntClear(GPIO_PORTC_BASE, GPIO_PIN_4);
}

// Quadrature encoder interrupt
static void yawIntHandler(pin_reference_state)
{
      static int prev_pin_state = 0;
      int8_t pin_state = GPIOPinRead(GPIO_PORTB_BASE, INT_PINS);

      GPIOIntClear(GPIO_PORTB_BASE, INT_PINS);

      yaw += QDE[prev_pin_state][pin_state];
      prev_pin_state = pin_state;

      yaw_hund_deg_abs = yaw * 100 * DEGREES_PER_REV / TRANSITIONS_PER_REV;
      yaw_hund_deg = getYawWrap(yaw_hund_deg_abs, 100);

}

int32_t getYawHundDeg(void)
{
    return yaw_hund_deg;
}

int32_t getYawWrap(int32_t yaw_deg_abs, int32_t scale)
{
    return sign(yaw_deg_abs) * ((abs(yaw_deg_abs) + (180 * scale)) % (360 * scale) - (180 * scale));
}

// Retrieves the raw data of yaw
int32_t getYawRaw(void)
{
    return yaw;
}


//int32_t yawReference(int32_t yaw_deg)
//{
//    uint32_t wrapped_yaw = yaw_deg;
//
//    if (abs(yaw_deg) > 180) {
//        if yaw_deg < 0 {
//            // Negative yaw
//            wrapped_yaw = ((yaw_deg % 180) + 180);
//        } else {
//            //Positive yaw
//            wrapped_yaw = ((yaw_deg % 180) - 180);
//        }
//    }
//    return wrapped_yaw;
//}

int32_t getReferencePosition(int32_t yaw_hund_deg)
{
    int states = 1;
    int8_t pin_reference_state = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_4);
    int yaw_deg = sign(yaw_hund_deg) * abs(yaw_hund_deg) / 100;

    if (pin_reference_state != 0) {
        return yaw_deg
    } else {
        while (states < 3) {
            if (pin_reference_state = 0) {
                yaw_deg += 120;
                state += 1;
                //put some interrupt here
            } else {
                break;
            }
        }
        return yaw_deg;
    }

    elseif (pin_reference_state != 0) {

    }

    return yaw_deg;
}

// Initialises GPIO ports and pins to read yaw
void initYaw (void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, INT_PINS);
    GPIOPadConfigSet(GPIO_PORTB_BASE, INT_PINS, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
    GPIOIntTypeSet(GPIO_PORTB_BASE, INT_PINS, GPIO_BOTH_EDGES);
    GPIOIntRegister(GPIO_PORTB_BASE, yawIntHandler);
    GPIOIntEnable(GPIO_PORTB_BASE, INT_PINS);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOIntRegister(GPIO_PORTC_BASE, referenceIntHandler);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_PIN_4);

}

