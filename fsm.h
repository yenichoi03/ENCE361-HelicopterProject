//*****************************************************************************
//
// altitude.c - Calculates altitude from ADC
//
//         The samples (ADC outputs) are stored continually in a circular buffer. At regular intervals the mean value of the samples in the buffer are computed.
//
//
// Author:  ych227, sli219
//
//
//*****************************************************************************

#ifndef FSM_H
#define FSM_H

typedef enum {
    LANDED,
    CALIBRATING,
    TAKING_OFF,
    FLYING,
    LANDING, 
} heli_states_t;


#endif






