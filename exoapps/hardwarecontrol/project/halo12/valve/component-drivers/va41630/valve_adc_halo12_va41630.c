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

#include <stddef.h>
#include "adc/hal_adc.h"
#include "valve_adc.h"
#include "valve_mcu.h"

// ADC hardware channels
#define ANODE_PRESSURE   2
#define CAT_PRESSURE     3
#define TANK_PRESSURE    6
#define REG_PRESSURE     7
#define CAT_HF_VOLTAGE   4
#define TEMPERATURE      5

#define ZERO_PRESSURE_OFFSET            1008
#define CATH_LF_ZERO_PRESSURE_OFFSET    1120 // Anson added to account for different offsets

void valve_adc_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    adc_channel_register(CAT_HF_VOLTAGE, NULL);
    adc_channel_register(TEMPERATURE, NULL);
    adc_channel_register(TANK_PRESSURE, NULL);
    adc_channel_register(REG_PRESSURE, NULL);
    adc_channel_register(ANODE_PRESSURE, NULL);
    adc_channel_register(CAT_PRESSURE, NULL);
    
    adc_start();
}

void valve_adc_deinit(void)
{
    adc_channel_deregister(CAT_HF_VOLTAGE);
    adc_channel_deregister(TEMPERATURE);
    adc_channel_deregister(TANK_PRESSURE);
    adc_channel_deregister(REG_PRESSURE);
    adc_channel_deregister(ANODE_PRESSURE);
    adc_channel_deregister(CAT_PRESSURE);
}

void va_get_data(void)
{
    uint16_t pressure = adc_get_value(RETURN_MEDIAN, TANK_PRESSURE);
    if(ZERO_PRESSURE_OFFSET > pressure)
    {
        pressure = 0;
    } else {
        pressure -= ZERO_PRESSURE_OFFSET;
    }
    valve.tank_pressure = pressure;

    pressure = adc_get_value(RETURN_MEDIAN, CAT_PRESSURE);
    if(CATH_LF_ZERO_PRESSURE_OFFSET > pressure)
    {
        pressure = 0;
    } else {
        pressure -= CATH_LF_ZERO_PRESSURE_OFFSET;
    }
    valve.cathode_pressure = pressure;

    pressure = adc_get_value(RETURN_MEDIAN, ANODE_PRESSURE);
    if(ZERO_PRESSURE_OFFSET > pressure)
    {
        pressure = 0;
    } else {
        pressure -= ZERO_PRESSURE_OFFSET;
    }
    valve.anode_pressure = pressure;

    pressure = adc_get_value(RETURN_MEDIAN, REG_PRESSURE);
    if(ZERO_PRESSURE_OFFSET > pressure)
    {
        pressure = 0;
    } else {
        pressure -= ZERO_PRESSURE_OFFSET;
    }
    valve.regulator_pressure = pressure;

    valve.cat_high_flow_voltage = adc_get_value(RETURN_MEDIAN, CAT_HF_VOLTAGE);
    valve.temperature           = adc_get_value(RETURN_MEDIAN, TEMPERATURE);
}
