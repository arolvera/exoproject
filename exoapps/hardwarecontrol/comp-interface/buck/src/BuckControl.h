/* 
 * File:   BuckControl.h
 * Author: fnorwood
 *
 * Created on March 2, 2022, 3:23 PM
 */

#ifndef BUCKCONTROL_H
#define	BUCKCONTROL_H

#include "buck_mcu.h"
#include "stateHandlers.h"

void BuckControlHandler(void);
void Control_init(void);

// Data structure for all global variables
typedef struct
{
    buck_state_t CurrentState;
    uint16_t DACOutput;
    uint16_t OutputCurrent;
    uint16_t OutputVoltage;
    uint16_t InputCurrent;
    uint16_t InputVoltage;
    buck_error_t ErrorCode;
    uint16_t ErrorADC;
}Control_t;

volatile Control_t Control;     // Global variable space

#endif	/* BUCKCONTROL_H */

