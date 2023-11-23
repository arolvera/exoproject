/*
           Copyright (C) 2022 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file via any medium is strictly prohibited.
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this 
 software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#include "msg-handler/msg_handler.h"
#include "component_communication.h"
#include "component_service.h"
#include "mcu_include.h"
#include "hardwarecontrol_version.h"



/**
 * Copy a health entry to a buffer
 * @param entry pointer to the health entry you want copied
 * @param buf pointer to the buffer you want the entry copied to
 * @return the number of bytes copied
 */
static int cc_copy_health(const health_table_entry_t *entry, uint8_t *buf)
{
    int bytes_copied = 0;
    uint8_t *pBuf = buf;
    const health_table_entry_t *pEntry = entry;
    uint8_t size= pEntry->size;
    const volatile uint8_t *data = pEntry->data;
    while(size != HEALTH_ENTRY_SIZE_EOL)
    {
        while(size--)
        {
            *pBuf++ = *data++;
            bytes_copied++;
        }
        pEntry++;
        data = pEntry->data;
        size = pEntry->size;
    }
    return bytes_copied;
}

void cc_sync(uint8_t *fifo, health_array health, uint8_t id)
{
    uint8_t buf[MAX_MSG_SIZE] = {0};
    uint8_t entry = *fifo; // Read the first byte - should be health sync
    if(entry < HEALTH_ENTRIES) {
        if(entry == CAN_HSI_SYNC) {
            //@fixme add the CAN health back in
            //health_table_entry_t *p = can_health_get();
            //cc_copy_health(p, buf);
        } else {
            cc_copy_health(&health[entry][0], buf);
        }
        send_msg((HEALTH_ID_BASE | id), buf, 8, 1000);
    }
}


void cc_initial_boot_report(uint8_t id)
{
    communication_union_t msg = { .data ={0,0,0,0,0,0,0,0} };
    msg.bcast_initial_boot.bcast_type = BCAST_INITIAL_BOOT;
    send_msg((BROADCAST_STATE_ID_BASE | id), msg.data, CAN_DLC_8, 1000);
}

void cc_error_report(uint8_t error, uint16_t adc_val, uint8_t id)
{
    communication_union_t msg = { .data ={0,0,0,0,0,0,0,0} };
    msg.bcast_error.bcast_type = BCAST_ERROR;
    msg.bcast_error.error_code = error;
    msg.bcast_error.adc_val    = adc_val;
    send_msg((BROADCAST_STATE_ID_BASE | id), msg.data, CAN_DLC_8, 1000);
}

void cc_state_change_report(uint8_t state, uint8_t reason, uint8_t id)
{
    communication_union_t msg = { .data ={0,0,0,0,0,0,0,0} };
    msg.bcast_state_change.bcast_type = BCAST_STATE_CHANGE;
    msg.bcast_state_change.state      = state;
    msg.bcast_state_change.reason     = reason;
    send_msg((BROADCAST_STATE_ID_BASE | id), msg.data, CAN_DLC_8, 1000);
}

void cc_version_report(uint8_t id)
{
    communication_union_t msg = { .data ={0,0,0,0,0,0,0,0} };

    msg.bcast_version.type          = VERSION_INFO;
    msg.bcast_version.major         = HW_MAJOR_VERSION;
    msg.bcast_version.minor         = HW_MINOR_VERSION;
    msg.bcast_version.rev           = HW_REVISION;
    msg.bcast_version.git_sha       = HW_GIT_SHA;

    send_msg((BROADCAST_VARIABLE_ID_BASE | id), msg.data, CAN_DLC_8, 1000);
}

void cc_scaling_report(float scaling, uint8_t description, uint8_t id)
{
    communication_union_t msg = { .data ={0,0,0,0,0,0,0,0} };
    msg.bcast_scaling.type             = SCALING_INFO;
    msg.bcast_scaling.scaling          = scaling;
    msg.bcast_scaling.unit_description = description;
    send_msg((BROADCAST_VARIABLE_ID_BASE | id), msg.data, CAN_DLC_8, 1000);
}

/**
 * Processes a command and calls the relevant function for that command
 * @param table pointer to the command table you want the data compared to 
 * @param data pointer to the raw command data
 */
void cc_command_processor(command_table_t *table, uint8_t *data, uint8_t id)
{
    // echo...echo...echo...echo...
    communication_union_t *msg = (communication_union_t*)data;
    //ACK
    send_msg((RESPONSE_PARAMETERS_ID_BASE | id), msg->data, CAN_DLC_8, 1000);
    
    int n = 0;
    CMD_t cmd = NULL;

    while(n < table->n_commands && cmd == NULL)
    {
        if(data[0] == table->CMDS[n].command)
        {
            cmd = table->CMDS[n].CMD;
        } else {
            n++;
        }
    }
    if(cmd == NULL)
    {
        cc_error_report(INVALID_COMMAND, 0, id);
    } else {
        cmd(data);
    }
}

