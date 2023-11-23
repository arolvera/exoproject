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
#include "magnet_control.h"
#include "magnet_event_handlers.h"
#include "magnet_hardware_port.h"
#include "magnet_pwm.h"
#include "magnet_state_handlers.h"
#include <stdbool.h>
#include <stddef.h>

states_t magnet_on_command_handler(void)
{
    mhp_enable();
    msh_reset_counters();
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_ON, common.error_ADC);
    return STARTUP_STATE;
}

states_t magnet_power_good_handler(void)
{
    mhp_LED_on();
    cc_send_message(BCAST_OUTPUT_CONTROL, POWER_GOOD, common.error_ADC);
    return ON_STATE;
}

states_t magnet_off_command_handler(void)
{
    mhp_disable();
    magnet.inner_PWM = 0;
    magnet.outer_PWM = 0;
    mp_set_inner_PWM(magnet.inner_PWM);
    mp_set_outer_PWM(magnet.outer_PWM);
    mhp_LED_off();
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_OFF, common.error_ADC);
    return OFF_STATE;
}

states_t magnet_error_handler(void)
{
    mhp_disable();
    magnet.inner_PWM = 0;
    magnet.outer_PWM = 0;
    mp_set_inner_PWM(magnet.inner_PWM);
    mp_set_outer_PWM(magnet.outer_PWM);
    mhp_LED_off();
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return ERROR_STATE;
}

states_t magnet_error_cleared_handler(void)
{
    common.error_code = MAGNET_NO_ERROR;
    common.error_ADC = 0;
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return OFF_STATE;
}
