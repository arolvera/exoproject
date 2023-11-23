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

#include "common_initialize.h"
#include "component_communication.h"
#include "component_service.h"
#include "magnet_adc.h"
#include "magnet_control.h"
#include "magnet_hardware_port.h"
#include "magnet_mcu.h"
#include "magnet_pwm.h"
#include "magnet_timer.h"
#include <stddef.h>

void magnet_initialize(void)
{    
    MCU_CLOCK_PRESCALER_CONFIG;
    
    magnet_hardware_port_init();
        
    magnet_PSC_init();
    
    magnet_control_init();

    common_init(CAN_ID_MAGNET, mc_sync);
    
    cc_scaling_report(MAGNET_COUNTS_PER_VOLT, COUNTS_PER_VOLT_OUTPUT);
    cc_scaling_report(MAGNET_COUNTS_PER_AMPERE, COUNTS_PER_AMPERE_OUTPUT);
    cc_scaling_report(MAGNET_COUNTS_PER_DEGREE_CELCIUS, TEMPERATURE_LINEAR);
    
    magnet_ADC_init();
    
    magnet_timer_init();
    
    common.current_state = common_init_complete();
}