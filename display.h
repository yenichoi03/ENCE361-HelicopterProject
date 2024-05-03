/**  @file   yaw.c
     @author Yeni Choi, Steven Little
     @date   14 Oct 2023
     @brief  gives  reliable continuous yaw monitoring with sub-degree precision.

             The program should calculate yaw in degrees, relative to the initial orientation of the portable mount when program execution starts.
             When viewed from above, clockwise rotation should correspond to positive yaw, counter-clockwise to negative

*/

#ifndef DISPLAY_H
#define DISPLAY_H


typedef enum displayMode {HEIGHT = 0, FILTERED, OFF, YAW} displayMode_t;
#define DISPLAY_MODES 4


// Initialises the display
void initDisplay (void);

// Function to display the filtered ADC value (10-bit value, note) and sample count.
void displayStatistics(uint16_t filteredVal, uint16_t currentVal, int16_t heightPercent, uint32_t count, displayMode_t mode, int yaw, int yaw_hund_deg);

#endif





