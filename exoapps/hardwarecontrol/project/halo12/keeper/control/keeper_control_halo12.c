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

#include "component_communication.h"
#include "component_service.h"
#include "keeper_control.h"
#include "keeper_mcu.h"


static health_table_entry_t health_response_0[] = 
    DECLARE_KEEPER_HEALTH_ENTRY_0(&keeper.flyback_voltage,
                                  &keeper.starter_voltage,
                                  &keeper.output_current);

static health_table_entry_t health_response_1[] = 
    DECLARE_KEEPER_HEALTH_ENTRY_1(&keeper.common.current_state,
                                  &keeper.common.error_code,
                                  &keeper.pwm_output);

static health_table_entry_t health_response_2[] =
    DECLARE_KEEPER_HEALTH_ENTRY_2(&keeper.temperature);

static health_array keeper_health = {
    health_response_0,
    health_response_1,
    health_response_2
};

int kc_sync(message_t *msg)
{
    cc_sync(msg->data, keeper_health, COMM_ID_KEEPER);
    return 0;
}

void keeper_control_init(void)
{
    keeper.target_voltage = 0;
    keeper.target_current = 0;
    keeper.common.error_code = KEEPER_NO_ERROR;
    keeper.common.error_adc = 0;
    keeper.pwm_output = 0;
}

states_t keeper_state_get(void)
{
    return keeper.common.current_state;
}