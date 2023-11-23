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

#include "anode_control.h"
#include "anode_mcu.h"
#include "component_communication.h"
#include "msg_callback.h"

static health_table_entry_t health_response_0[] = 
    DECLARE_ANODE_HEALTH_ENTRY_0(&anode.output_voltage,    
                                 &anode.output_current,
                                 &anode.x_voltage,
                                 &anode.y_voltage);

static health_table_entry_t health_response_1[] = 
    DECLARE_ANODE_HEALTH_ENTRY_1(&anode.common.current_state,
                                 &anode.common.error_code,
                                 &anode.x_pwm_output,
                                 &anode.y_pwm_output,
                                 &anode.mode);

static health_table_entry_t health_response_2[] =
    DECLARE_ANODE_HEALTH_ENTRY_2(&anode.temperature,
                                 &anode.raw_input_voltage,
                                 &anode.filtered_input_voltage);

static health_array anode_health = {
    health_response_0,
    health_response_1,
    health_response_2
};

int ac_sync(message_t *msg)
{
    cc_sync(msg->data, anode_health, COMM_ID_ANODE);
    return 0;
}

void anode_control_init(void)
{
    anode.target_voltage = 0;
    anode.common.error_code = ANODE_NO_ERROR;
    anode.common.error_adc = 0;
    anode.x_pwm_output = 0;
    anode.y_pwm_output = 0;
    anode.mode = QUADRATIC_BOOST;
}

states_t anode_state_get(void)
{
    return anode.common.current_state;
}