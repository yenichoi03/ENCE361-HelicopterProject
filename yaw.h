/**  @file   yaw.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/

#ifndef YAW_H
#define YAW_H

// CONSTANTS
#define TRANSITIONS_PER_REV (WHEEL_SLOTS * 4)
#define DEGREES_PER_REV 360
#define INT_PINS GPIO_PIN_0 | GPIO_PIN_1
#define WHEEL_SLOTS 112

// GLOBAL VARIABLES
int yaw = 0;
int yaw_hund_deg = 0;


/**Quadrature encoder coding yaw into degrees**/
void GPIOIntHandler(void);

/**Calls the interrupt handler for every rising or falling edge detected**/
void initGPIO (void);

#endif
