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
#include "magnet_control.h"
#include "magnet_mcu.h"

static health_table_entry_t health_response_0[] = 
    DECLARE_MAGNET_HEALTH_ENTRY_0(&magnet.output_current,
                                  &magnet.output_voltage);

static health_table_entry_t health_response_1[] = 
    DECLARE_MAGNET_HEALTH_ENTRY_1(&magnet.common.current_state,
                                  &magnet.common.error_code,
                                  &magnet.pwm_output);

static health_table_entry_t health_response_2[] =
    DECLARE_MAGNET_HEALTH_ENTRY_2(&magnet.temperature);

static health_array magnet_health = {
    health_response_0,
    health_response_1,
    health_response_2
};

int mc_sync(message_t *msg)
{
    cc_sync(msg->data, magnet_health, COMM_ID_MAGNET);
    return 0;
}

void magnet_control_init(void)
{
    magnet.common.error_code = MAGNET_NO_ERROR;
    magnet.common.error_adc = 0;
    magnet.pwm_output = 0;
    magnet.target_current = 0;
}

states_t magnet_state_get(void)
{
    return magnet.common.current_state;
}
