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
#include "anode_comp.h"
#include "anode_control.h"
#include "anode_hardware_port.h"
#include "anode_pwm.h"
#include "anode_timer.h"
#include "common_DAC.h"
#include "common_initialize.h"
#include "component_communication.h"
#include "component_service.h"
#include <stddef.h>

void anode_initialize(void)
{    
    MCU_CLOCK_PRESCALER_CONFIG;
    
    anode_hardware_port_init();
    
    anode_PSC_init();
    
    common_DAC_init();
    
    anode_comp_init();
    
    anode_control_init();
    
    common_init(CAN_ID_ANODE, ac_sync);
    
    cc_scaling_report(ANODE_COUNTS_PER_VOLT, COUNTS_PER_VOLT_OUTPUT);
    cc_scaling_report(ANODE_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT);
    
    anode_ADC_init();
    
    anode_timer_init();
    
    common.current_state = common_init_complete();
}