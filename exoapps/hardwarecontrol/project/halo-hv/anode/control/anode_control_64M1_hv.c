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

#include "anode_adc.h"
#include "anode_command_handlers.h"
#include "anode_control.h"
#include "anode_state_handlers.h"
#include "anode_timer.h"
#include "component_communication.h"
#include "component_service.h"
#include <stddef.h>

static health_table_entry_t health_response_0[] = 
    DECLARE_ANODE_HEALTH_ENTRY_0(&anode.output_voltage,    
                                 &anode.output_current);

static health_table_entry_t health_response_1[] = 
    DECLARE_ANODE_HEALTH_ENTRY_1(&anode.PWM_output,
                                 &common.current_state,
                                 &common.error_code);


static health_array anode_health = {
    health_response_0,
    health_response_1
};

void ac_sync(uint32_t id, uint8_t dlc, volatile uint8_t *fifo)
{
    cc_sync(CAN_ID_ANODE, dlc, fifo, anode_health);
}

void anode_control_init(void)
{
    anode.target_voltage = 0;
    common.error_code = ANODE_NO_ERROR;
    common.error_ADC = 0;
    anode.PWM_output = 0;
}
