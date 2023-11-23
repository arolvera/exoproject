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
#include "anode_control.h"
#include "anode_hardware_port.h"
#include "anode_mcu.h"
#include "anode_pwm.h"
#include "anode_timer.h"
#include "component_communication.h"

void anode_initialize(void)
{
    anode_hardware_port_init();

    anode_control_init();
    
    anode_pwm_init();

    anode_timer_init();

    cc_initial_boot_report(COMM_ID_ANODE);

    cc_version_report(COMM_ID_ANODE);
    cc_scaling_report(ANODE_COUNTS_PER_VOLT_OUTPUT, COUNTS_PER_VOLT_OUTPUT, COMM_ID_ANODE);

    cc_scaling_report(ANODE_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT, COMM_ID_ANODE);
    cc_scaling_report(ANODE_COUNTS_PER_VOLT_INPUT, COUNTS_PER_VOLT_INPUT, COMM_ID_ANODE);

    anode_adc_init();

    anode.common.current_state = cs_init_complete(COMM_ID_ANODE);
}

// ToDo: not used, can we get rid of it, or is there unfinished work?
void anode_shutdown(void)
{
    ahp_input_power_enable(false);
    anode.x_pwm_output = 0;
    anode.y_pwm_output = 0;
    ap_set_pwm(anode.x_pwm_output, X_PWM);
    ap_set_pwm(anode.y_pwm_output, Y_PWM);
    ahp_spark_detect_enable(false);

    anode_adc_deinit();
    anode_timer_deinit();
}
