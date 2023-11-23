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
#include "dac/hal_dac.h"
#include "valve_adc.h"
#include "valve_control.h"
#include "valve_hardware_port.h"
#include "valve_mcu.h"
#include "valve_pwm.h"
#include "valve_timer.h"


void valve_initialize(void)
{
    valve_hardware_port_init();

    valve_control_init();

    valve_pwm_init();

    valve_timer_init();


    dac_common_init(CATHODE_LF_DAC);
    dac_common_init(ANODE_FLOW_DAC);

    cc_initial_boot_report(COMM_ID_VALVE);
    cc_version_report(COMM_ID_VALVE);
    cc_scaling_report(VALVE_LOW_PRESSURE_SENSOR_SCALE, COUNTS_PER_PSI_LOW_PRESSURE, COMM_ID_VALVE);
    cc_scaling_report(VALVE_HIGH_PRESSURE_SENSOR_SCALE, COUNTS_PER_PSI_HIGH_PRESSURE, COMM_ID_VALVE);
    cc_scaling_report(VALVE_VOLTAGE_SCALE, COUNTS_PER_VOLT_OUTPUT, COMM_ID_VALVE);
    cc_scaling_report(VALVE_THERMISTOR_BETA, THERMISTOR_BETA, COMM_ID_VALVE);
    cc_scaling_report(VALVE_THERMISTOR_R_NOUGHT, THERMISTOR_R_NOUGHT, COMM_ID_VALVE);
    
    valve_adc_init();
    
    vhp_lv_close();
    
    valve.common.current_state = cs_init_complete(COMM_ID_VALVE);
}

// ToDo: not used, can we get rid of it, or is there unfinished work?
void valve_shutdown(void)
{
    vhp_lv_close();
    vp_set_pwm(0);
    valve.cat_low_flow_dac = 0;
    dac_set(valve.cat_low_flow_dac, CATHODE_LF_DAC);
    valve.anode_dac = 0;
    dac_set(valve.anode_dac, ANODE_FLOW_DAC);

    valve_adc_deinit();
    valve_timer_deinit();
}