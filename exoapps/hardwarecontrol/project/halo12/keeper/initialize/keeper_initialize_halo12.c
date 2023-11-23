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
#include "keeper_adc.h"
#include "keeper_control.h"
#include "keeper_hardware_port.h"
#include "keeper_mcu.h"
#include "keeper_pwm.h"
#include "keeper_timer.h"
#include "keeper_state_handlers.h"

void keeper_initialize(void)
{
    keeper_hardware_port_init();
    
    keeper_control_init();

    //keeper_pwm_init();

    //keeper_timer_init();

    cc_initial_boot_report(COMM_ID_KEEPER);
    cc_version_report(COMM_ID_KEEPER);
    cc_scaling_report(KEEPER_COUNTS_PER_VOLT_FLYBACK, COUNTS_PER_VOLT_OUTPUT, COMM_ID_KEEPER);
    cc_scaling_report(KEEPER_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT, COMM_ID_KEEPER);

    //keeper_adc_init();
    
    keeper.common.current_state = cs_init_complete(COMM_ID_KEEPER);
}

// ToDo: not used, can we get rid of it, or is there unfinished work?
void keeper_shutdown(void)
{
    keeper_starter_enable(false);
    keeper.pwm_output = 0;
    kp_set_flyback_pwm(keeper.pwm_output);
    khp_spark_detect_enable(false);

    keeper_adc_deinit();
    keeper_timer_deinit();
}