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

#include "common_DAC.h"
#include "component_communication.h"
#include "keeper_comp.h"
#include "keeper_control.h"
#include "keeper_event_handlers.h"
#include "keeper_hardware_port.h"
#include "keeper_state_handlers.h"
#include <stddef.h>

#define DAC_START_POINT    500

states_t keeper_on_command_handler(void)
{
    ksh_reset_counters();
    khp_enable();
    kc_spark_detect_enable();    
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_ON, common.error_ADC);
    return STARTUP_STATE;
}

states_t keeper_spark_detected_handler(void)
{
    kc_spark_detect_disable();
    keeper.DAC_output = DAC_START_POINT;
    cd_set_DAC(keeper.DAC_output);
    ksh_reset_counters();
    cc_send_message(BCAST_OUTPUT_CONTROL, SPARK_DETECTED, common.error_ADC);
    return ON_STATE;
}

states_t keeper_off_command_handler(void)
{
    khp_disable();
    keeper.DAC_output = 0;
    cd_set_DAC(keeper.DAC_output);
    kc_spark_detect_disable();
    cc_send_message(BCAST_OUTPUT_CONTROL, COMMANDED_OFF, common.error_ADC);
    return OFF_STATE;
}

states_t keeper_error_handler(void)
{
    khp_disable();
    keeper.DAC_output = 0;
    cd_set_DAC(keeper.DAC_output);
    kc_spark_detect_disable();
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return ERROR_STATE;
}

states_t keeper_error_cleared_handler(void)
{
    common.error_code = KEEPER_NO_ERROR;
    common.error_ADC = 0;
    cc_send_message(BCAST_ERROR, common.error_code, common.error_ADC);
    return OFF_STATE;
}