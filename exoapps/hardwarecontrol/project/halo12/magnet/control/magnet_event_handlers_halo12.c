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
#include "magnet_event_handlers.h"
#include "magnet_mcu.h"
#include "magnet_pwm.h"
#include "magnet_state_handlers.h"

states_t magnet_on_command_handler(void)
{
    msh_reset_counters();
    cc_state_change_report(STARTUP_STATE, COMMANDED_ON, COMM_ID_MAGNET);
    return STARTUP_STATE;
}

states_t magnet_power_good_handler(void)
{
    cc_state_change_report(ON_STATE, POWER_GOOD, COMM_ID_MAGNET);
    return ON_STATE;
}

states_t magnet_off_command_handler(void)
{
    magnet.pwm_output = 0;
    mp_set_pwm(magnet.pwm_output);
    cc_state_change_report(OFF_STATE, COMMANDED_OFF, COMM_ID_MAGNET);
    return OFF_STATE;
}

states_t magnet_error_handler(void)
{
    magnet.pwm_output = 0;
    mp_set_pwm(magnet.pwm_output);
    cc_error_report(magnet.common.error_code, magnet.common.error_adc, COMM_ID_MAGNET);
    cc_state_change_report(ERROR_STATE, ERROR_DETECTED, COMM_ID_MAGNET);
    return ERROR_STATE;
}

states_t magnet_error_cleared_handler(void)
{
    magnet.common.error_code = MAGNET_NO_ERROR;
    magnet.common.error_adc = 0;
    cc_error_report(magnet.common.error_code, magnet.common.error_adc, COMM_ID_MAGNET);
    cc_state_change_report(OFF_STATE, ERROR_CLEARED, COMM_ID_MAGNET);
    return OFF_STATE;
}
