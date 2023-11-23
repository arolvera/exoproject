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
#include "common_initialize.h"
#include "component_communication.h"
#include "component_service.h"
#include "keeper_adc.h"
#include "keeper_comp.h"
#include "keeper_control.h"
#include "keeper_hardware_port.h"
#include "keeper_mcu.h"
#include <stddef.h>

void keeper_initialize(void)
{
    MCU_CLOCK_PRESCALER_CONFIG;
    
    keeper_hardware_port_init();
    
    common_DAC_init();
    
    keeper_comp_init();
    
    keeper_control_init();
    
    common_init(CAN_ID_KEEPER, kc_sync);
    
    cc_scaling_report(KEEPER_COUNTS_PER_VOLT, COUNTS_PER_VOLT_OUTPUT);
    cc_scaling_report(KEEPER_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT);
    
    keeper_ADC_init();
    
    common.current_state = common_init_complete();
}