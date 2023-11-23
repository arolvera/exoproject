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
#include "valve_adc.h"
#include "valve_control.h"
#include "valve_hardware_port.h"
#include "valve_mcu.h"
#include "valve_pwm.h"
#include "valve_timer.h"
#include <stddef.h>

void valve_initialize(void)
{    
    MCU_CLOCK_PRESCALER_CONFIG;
    
    valve_PSC_init();
    
    valve_hardware_port_init();
    
    valve_timer_init();
    
    valve_control_init();
    
    common_init(CAN_ID_VALVE, vc_sync);
    
    cc_scaling_report(VALVE_40_PSI_SENSOR_SCALE, COUNTS_PER_PSI_FORTY);
    cc_scaling_report(VALVE_3000_PSI_SENSOR_SCALE, COUNTS_PER_PSI_THREE_THOUSAND);
    cc_scaling_report(VALVE_VOLTAGE_SCALE, COUNTS_PER_VOLT_OUTPUT);
    cc_scaling_report(VALVE_THERMISTOR_BETA, THERMISTOR_BETA);
    cc_scaling_report(VALVE_THERMISTOR_R_NOUGHT, THERMISTOR_R_NOUGHT);
    
    valve_ADC_init();
    
    vhp_lv_close();
    
    common.current_state = common_init_complete();
}