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
#include "magnet_adc.h"
#include "magnet_control.h"
#include "magnet_hardware_port.h"
#include "magnet_mcu.h"
#include "magnet_pwm.h"
#include "magnet_timer.h"


void magnet_initialize(void)
{
    magnet_hardware_port_init();

    magnet_control_init();

    magnet_pwm_init();

    magnet_timer_init();


    cc_initial_boot_report(COMM_ID_MAGNET);
    cc_version_report(COMM_ID_MAGNET);
    cc_scaling_report(MAGNET_COUNTS_PER_VOLT, COUNTS_PER_VOLT_OUTPUT, COMM_ID_MAGNET);
    cc_scaling_report(MAGNET_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT, COMM_ID_MAGNET);
    
    magnet_adc_init();

    magnet.common.current_state = cs_init_complete(COMM_ID_MAGNET);
}

// ToDo: not used, can we get rid of it, or is there unfinished work?
void magnet_shutdown(void)
{
    magnet.pwm_output = 0;
    mp_set_pwm(magnet.pwm_output);

    magnet_adc_deinit();
    magnet_timer_deinit();
}