/* 
 * File:   eventHandlers.c
 * Author: fnorwood
 *
 * Created on March 10, 2022, 4:10 PM
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
#include "eventHandlers.h"
#include "COMP_M64.h"
#include "commsHandler_M64.h"

buck_state_t onCommandHandler(void)
{
    BUCK_ENABLE;
    OVER_I_COMP_ENABLE;
    Send_SAM_reply(BCAST_OUTPUT_CONTROL, COMMANDED_ON, Control.ErrorADC);
    return SOFT_START_STATE_BUCK;
}

buck_state_t powerGoodHandler(void)
{
    LED_ON;
    Send_SAM_reply(BCAST_OUTPUT_CONTROL, POWER_GOOD, Control.ErrorADC);
    return ON_STATE_BUCK;
}

buck_state_t offCommandHandler(void)
{
    BUCK_DISABLE;
    Control.DACOutput = 0;
    setDAC(Control.DACOutput);
    OVER_I_COMP_DISABLE;
    LED_OFF;
    Send_SAM_reply(BCAST_OUTPUT_CONTROL, COMMANDED_OFF, Control.ErrorADC);
    return OFF_STATE_BUCK;
}

buck_state_t errorHandler(void)
{
    OVER_I_COMP_DISABLE;
    BUCK_DISABLE;
    Control.DACOutput = 0;
    setDAC(Control.DACOutput);
    LED_OFF;
    Send_SAM_reply(BCAST_ERROR, Control.ErrorCode, Control.ErrorADC);
    return ERROR_STATE_BUCK;
}

buck_state_t errorClearedHandler(void)
{
    Control.ErrorCode = NO_ERROR_BUCK;
    Control.ErrorADC = 0;
    Send_SAM_reply(BCAST_ERROR, Control.ErrorCode, Control.ErrorADC);
    return OFF_STATE_BUCK;
}