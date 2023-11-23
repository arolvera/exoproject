/* 
 * File:   stateHandlers.c
 * Author: fnorwood
 *
 * Created on March 10, 2022, 4:04 PM
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <stdbool.h>
#include "HardwarePort.h"
#include "DAC_M64.h"
#include "BuckControl.h"
#include "buck_mcu.h"
#include "stateHandlers.h"
#include "eventHandlers.h"

// safe operating limits and targets
#define MAX_OUTPUT_CURRENT         4.2  // Amperes
#define MAX_OUTPUT_VOLTAGE         32   // Volts
#define MIN_OUTPUT_VOLTAGE         24   // Volts
#define MAX_INPUT_CURRENT          1.7  // Amperes
#define MAX_INPUT_VOLTAGE          110  // Volts
#define MIN_INPUT_VOLTAGE          70   // Volts
#define TARGET_OUTPUT_VOLTAGE      28   // Volts
#define POWER_GOOD_RANGE           0.1  // Volts

// safe operating limits and targets in counts
#define MAX_I_OUT_COUNTS           MAX_OUTPUT_CURRENT * BUCK_OUTPUT_COUNTS_PER_AMPERE
#define MAX_V_OUT_COUNTS           MAX_OUTPUT_VOLTAGE * BUCK_OUTPUT_COUNTS_PER_VOLT
#define MIN_V_OUT_COUNTS           MIN_OUTPUT_VOLTAGE * BUCK_OUTPUT_COUNTS_PER_VOLT
#define MAX_I_IN_COUNTS            MAX_INPUT_CURRENT * BUCK_INPUT_COUNTS_PER_AMPERE
#define MAX_V_IN_COUNTS            MAX_INPUT_VOLTAGE * BUCK_INPUT_COUNTS_PER_VOLT
#define MIN_V_IN_COUNTS            MIN_INPUT_VOLTAGE * BUCK_INPUT_COUNTS_PER_VOLT
#define TARGET_VOLTAGE_COUNTS      TARGET_OUTPUT_VOLTAGE * BUCK_OUTPUT_COUNTS_PER_VOLT
#define POWER_GOOD_RANGE_COUNTS    POWER_GOOD_RANGE * BUCK_OUTPUT_COUNTS_PER_VOLT
#define MAX_DAC_OUT                600

void powerCheckSoftStartState(void) 
{
    uint16_t trueOutputCurrent = Control.OutputCurrent - BUCK_ZERO_CURRENT_OFFSET;
    if (Control.OutputCurrent < BUCK_ZERO_CURRENT_OFFSET)
    {
        trueOutputCurrent = 0;
    } 
    uint16_t trueInputCurrent = Control.InputCurrent - BUCK_ZERO_CURRENT_OFFSET;
    if (Control.InputCurrent < BUCK_ZERO_CURRENT_OFFSET)
    {
        trueInputCurrent = 0;
    }
    
    if (trueOutputCurrent > MAX_I_OUT_COUNTS)
    { 
        Control.ErrorADC = Control.OutputCurrent;
        Control.ErrorCode = OUTPUT_CURRENT_ERROR_BUCK;
    }
    else if (Control.OutputVoltage > MAX_V_OUT_COUNTS) 
    {
        Control.ErrorADC = Control.OutputVoltage;
        Control.ErrorCode = OUTPUT_VOLTAGE_ERROR_BUCK;
    } 
    else if (trueInputCurrent > MAX_I_IN_COUNTS) 
    {
        Control.ErrorADC = Control.InputCurrent;
        Control.ErrorCode = INPUT_CURRENT_ERROR_BUCK;
    }
    else if ((Control.InputVoltage > MAX_V_IN_COUNTS) || (Control.InputVoltage < MIN_V_IN_COUNTS)) 
    {
        Control.ErrorADC = Control.InputVoltage;
        Control.ErrorCode = INPUT_VOLTAGE_ERROR_BUCK;
    }
    
    if(Control.ErrorCode != NO_ERROR_BUCK)
    {
        Control.CurrentState = errorHandler();
    }
}

void powerCheckOnState(void)
{
    uint16_t trueOutputCurrent = Control.OutputCurrent - BUCK_ZERO_CURRENT_OFFSET;
    if (Control.OutputCurrent < BUCK_ZERO_CURRENT_OFFSET)
    {
        trueOutputCurrent = 0;
    } 
    uint16_t trueInputCurrent = Control.InputCurrent - BUCK_ZERO_CURRENT_OFFSET;
    if (Control.InputCurrent < BUCK_ZERO_CURRENT_OFFSET)
    {
        trueInputCurrent = 0;
    }
    
    if (trueOutputCurrent > MAX_I_OUT_COUNTS) 
    {
        Control.ErrorADC = Control.OutputCurrent;
        Control.ErrorCode = OUTPUT_CURRENT_ERROR_BUCK;
    }
    else if ((Control.OutputVoltage > MAX_V_OUT_COUNTS) || (Control.OutputVoltage < MIN_V_OUT_COUNTS))
    {
        Control.ErrorADC = Control.OutputVoltage;
        Control.ErrorCode = OUTPUT_VOLTAGE_ERROR_BUCK;
    }
    else if (trueInputCurrent > MAX_I_IN_COUNTS) 
    {
        Control.ErrorADC = Control.InputCurrent;
        Control.ErrorCode = INPUT_CURRENT_ERROR_BUCK;
    }
    else if ((Control.InputVoltage > MAX_V_IN_COUNTS) || (Control.InputVoltage < MIN_V_IN_COUNTS)) 
    {
        Control.ErrorADC = Control.InputVoltage;
        Control.ErrorCode = INPUT_VOLTAGE_ERROR_BUCK;
    }
    
    if(Control.ErrorCode != NO_ERROR_BUCK)
    {
        Control.CurrentState = errorHandler();
    }
}

void errorClearCheck(void)
{
    // check if the readings have returned to nominal
    uint16_t trueOutputCurrent = Control.OutputCurrent - BUCK_ZERO_CURRENT_OFFSET;
    if (Control.OutputCurrent < BUCK_ZERO_CURRENT_OFFSET)
    {
        trueOutputCurrent = 0;
    }
    uint16_t trueInputCurrent = Control.InputCurrent - BUCK_ZERO_CURRENT_OFFSET;
    if (Control.InputCurrent < BUCK_ZERO_CURRENT_OFFSET)
    {
        trueInputCurrent = 0;
    }
    
    if((trueOutputCurrent < MAX_I_OUT_COUNTS) &&
            (Control.OutputVoltage < MAX_V_OUT_COUNTS) &&
            (trueInputCurrent < MAX_I_IN_COUNTS) &&
            (Control.InputVoltage < MAX_V_IN_COUNTS) &&
            (Control.InputVoltage > MIN_V_IN_COUNTS))
    {
        Control.CurrentState = errorClearedHandler();
    }    
}

void offStateHandler(void)
{
    // In the words of the Magic Conch, "Do nothing."
}

void softStartStateHandler(void)
{
    powerCheckSoftStartState();
    // only perform control if there was no error
    if(Control.ErrorCode == NO_ERROR_BUCK)
    {
        if(Control.OutputVoltage < TARGET_VOLTAGE_COUNTS)
        {
            if (Control.DACOutput < MAX_DAC_OUT) 
            {
                Control.DACOutput++;
                setDAC(Control.DACOutput);
            } else {
                Control.ErrorCode = DAC_RAILED_ERROR_BUCK;
                Control.CurrentState = errorHandler();
            }        
        } else {
            Control.CurrentState = powerGoodHandler();
        }
    }
}

void onStateHandler(void)
{
    powerCheckOnState();
    // only perform control if there was no error
    if(Control.ErrorCode == NO_ERROR_BUCK)
    {
        // highly sophisticated plus one minus one control
        if(Control.OutputVoltage < (TARGET_VOLTAGE_COUNTS - POWER_GOOD_RANGE_COUNTS))
        {
            if (Control.DACOutput < MAX_DAC_OUT) 
            {
                Control.DACOutput++;
                setDAC(Control.DACOutput);
            } else {
                Control.ErrorCode = DAC_RAILED_ERROR_BUCK;
                Control.CurrentState = errorHandler();
            }        
        }     
        else if(Control.OutputVoltage > (TARGET_VOLTAGE_COUNTS + POWER_GOOD_RANGE_COUNTS))
        {
            if (Control.DACOutput > 0) 
            {
                Control.DACOutput--;
                setDAC(Control.DACOutput);
            } 
        }
    }
}

void errorStateHandler(void)
{
    errorClearCheck();
}