//*****************************************************************************
//
// fsm.h - Defines helicopter state type to allow use by multiple modules
//
//
// Author:  ych227, sli219
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






