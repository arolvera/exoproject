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

#include "common_ADC.h"
#include "valve_adc.h"
#include "valve_control.h"
#include <stddef.h>

// ADC hardware channels
#define ANODE_F_VOLTAGE  9 
#define CAT_HF_VOLTAGE   2
#define CAT_LF_VOLTAGE   3
#define TEMPERATURE      7
#define TANK_PRESSURE    6
#define REG_PRESSURE     10
#define ANODE_PRESSURE   8
#define CAT_PRESSURE     5

void valve_ADC_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    ca_channel_register(ANODE_F_VOLTAGE, NULL);
    ca_channel_register(CAT_HF_VOLTAGE, NULL);
    ca_channel_register(CAT_LF_VOLTAGE, NULL);
    ca_channel_register(TEMPERATURE, NULL);
    ca_channel_register(TANK_PRESSURE, NULL);
    ca_channel_register(REG_PRESSURE, NULL);
    ca_channel_register(ANODE_PRESSURE, NULL);
    ca_channel_register(CAT_PRESSURE, NULL);
    
    common_ADC_init();
}

void va_get_data(void)
{
    valve.tank_pressure         = ca_get_value(RETURN_MEDIAN, TANK_PRESSURE);
    valve.cathode_pressure      = ca_get_value(RETURN_MEDIAN, CAT_PRESSURE);
    valve.anode_pressure        = ca_get_value(RETURN_MEDIAN, ANODE_PRESSURE);
    valve.regulator_pressure    = ca_get_value(RETURN_MEDIAN, REG_PRESSURE);
    valve.anode_flow_voltage    = ca_get_value(RETURN_MEDIAN, ANODE_F_VOLTAGE);
    valve.cat_high_flow_voltage = ca_get_value(RETURN_MEDIAN, CAT_HF_VOLTAGE);
    valve.cat_low_flow_voltage  = ca_get_value(RETURN_MEDIAN, CAT_LF_VOLTAGE);
    valve.temperature           = ca_get_value(RETURN_MEDIAN, TEMPERATURE);
}
