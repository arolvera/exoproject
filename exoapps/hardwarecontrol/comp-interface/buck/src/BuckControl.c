/* 
 * File:   BuckControl.c
 * Author: fnorwood
 *
 * Created on March 2, 2022, 3:23 PM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <stdbool.h>
#include "HardwarePort.h"
#include "ADC_M64.h"
#include "DAC_M64.h"
#include "TIMER_M64.h"
#include "BuckControl.h"
#include "stateHandlers.h"

// Typedef of state handler function pointer
typedef void (*buckStateHandler_t)(void);

// Array of state handler function pointers. Must be in the same order as the states
buckStateHandler_t buckStateHandler[] = 
{
    offStateHandler,
    softStartStateHandler,
    onStateHandler,
    errorStateHandler
};

void Control_init(void)
{
    BUCK_DISABLE;
    Control.ErrorCode = NO_ERROR_BUCK;
    Control.ErrorADC = 0;
    Control.DACOutput = 0;
    setDAC(Control.DACOutput);
    Control.CurrentState = OFF_STATE_BUCK;
}

void BuckControlHandler(void)
{
    if(timerFlagGet() != 0){         // Check for timer flag
        // Read ADC values
        Control.OutputCurrent = GetOutputCurrent(RETURN_MEDIAN);
        Control.OutputVoltage = GetOutputVoltage(RETURN_MEDIAN);
        Control.InputCurrent = GetInputCurrent(RETURN_MEDIAN);
        Control.InputVoltage = GetInputVoltage(RETURN_MEDIAN);
        if (buckStateHandler[Control.CurrentState] != NULL)
        {
            (*buckStateHandler[Control.CurrentState])();
        }        
 		timerFlagReset();           //Reset timer flag after running control
 	}
}