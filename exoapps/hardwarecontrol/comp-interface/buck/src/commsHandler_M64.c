/* 
 * File:   commsHandler_M64.c
 * Author: fnorwood
 *
 * Created on March 2, 2022, 2:45 PM
 */

#include "commsHandler_M64.h"
#include "BuckControl.h"
#include "CAN_ATmega64m1_HAL.h"
#include "buck_mcu.h"
#include "component_communication.h"
#include "eventHandlers.h"
#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>

CommandReply_t CommandReply;

static health_table_entry_t health_response_0[] = 
    DECLARE_BUCK_HEALTH_ENTRY_0(&Control.OutputCurrent, 
                                &Control.OutputVoltage, 
                                &Control.InputCurrent,    
                                &Control.InputVoltage);

static health_table_entry_t health_response_1[] = 
    DECLARE_BUCK_HEALTH_ENTRY_1(&Control.DACOutput,
                                &Control.CurrentState,
                                &Control.ErrorCode,
                                &Control.ErrorADC); 

static buck_health_array buck_health = {
    health_response_0,
    health_response_1,
    0,
};

/**
 * Copy health info into buffer - Return number of bytes copied
 * @param entry pointer to health entry
 * @param buf buffer to store data
 * @return number of bytes copied into buffer
 */
int copy_health_entry(health_table_entry_t *entry, uint8_t *buf)
{
    int bytes_copied = 0;
    uint8_t *pBuf = buf;
    health_table_entry_t *pEntry = entry;
    while(pEntry->size != HEALTH_ENTRY_SIZE_EOL) {
        switch(pEntry->size) {
            case HEALTH_ENTRY_SIZE_1:
                *((uint8_t*) pBuf)  = *((uint8_t *)(pEntry->data));
                break;
            case HEALTH_ENTRY_SIZE_2:
                *((uint16_t*) pBuf) = *((uint16_t *)(pEntry->data));
                break;
            case HEALTH_ENTRY_SIZE_4:
                *((uint32_t*) pBuf) = *((uint32_t *)(pEntry->data));
                break;
            case HEALTH_ENTRY_SIZE_EOL:
            default:
                /* Wait what happened? */
                break;
        }
        /* Increment bytes copied */
        bytes_copied += pEntry->size;
        /* Advance the buffer pointer */
        pBuf += pEntry->size;
        /* Next! */
        pEntry++;
    }
    return bytes_copied;
}

void buck_sync(uint32_t id, uint8_t dlc, volatile uint8_t *fifo)
{
    common_communication_sync(CAN_ID_BUCK, dlc, fifo, buck_health);
}

/******************************************************************
 * Send state change message to SAM
 *
 * @param State - the current state of the MCU
 * @param Status - the reason the MCU is in that state
 * @param adc - the ADC value that caused an error, if applicable
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
void Send_SAM_reply(buck_state_t State, uint8_t Status, uint16_t adc)
{
    communication_union_t msg = { .data ={0,0,0,0,0,0,0,0} };
    msg.bcast_state.state = State;
    msg.bcast_state.reason = Status;
    msg.bcast_state.adc_val = adc;
    canSendMsg(BROADCAST_STATE_CAN_ID_BUCK, msg.data);
}

/******************************************************************
 * Process the function that has been received 
 *
 * @param rxBuffer
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
static inline void commandProcess(communication_union_t *cmd)
{
    if(cmd->command.command == ON_OFF)
    {
        if(cmd->command.set_point == ON_SET_POINT)
        {
            // Only go ON if OFF
            if(Control.CurrentState == OFF_STATE_BUCK)
            {
                Control.CurrentState = onCommandHandler();
            } else {
                Send_SAM_reply(Control.CurrentState, BAD_COMMAND_ERROR_BUCK, 0);
            }
        }        
        else if(cmd->command.set_point == OFF_SET_POINT)
        {
            // Don't go OFF if in ERROR
            if(Control.CurrentState != ERROR_STATE_BUCK)
            {
                Control.CurrentState = offCommandHandler();
            } else {
                Send_SAM_reply(BCAST_ERROR, Control.ErrorCode, Control.ErrorADC);
            }
        } else {
            Send_SAM_reply(BCAST_ERROR, BAD_COMMAND_ERROR_BUCK, 0);
        }
    } else {
        Send_SAM_reply(BCAST_ERROR, BAD_COMMAND_ERROR_BUCK, 0);
    }
}

/******************************************************************
 * Main CAN comms Rx handler function 
 *
 * Parameters: none
 * Returns: none
 * 
 * Revision
 *  - NEW
 *****************************************************************/
void CANcommHandler(void)
{
    communication_union_t cmd;
    /* Check for msg ready */
    bool msgReady = command_rx(cmd.data);
    if(msgReady) {
        /* Send back same msg */
        canSendMsg(RESPONSE_PARAMETERS_CAN_ID_BUCK, cmd.data);
        
        /* Process packet */
        commandProcess(&cmd);
    }   
}