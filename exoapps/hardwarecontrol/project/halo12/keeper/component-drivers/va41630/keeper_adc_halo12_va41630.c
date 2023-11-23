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

#include "adc/hal_adc.h"
#include "keeper_adc.h"
#include "keeper_event_handlers.h"
#include "keeper_mcu.h"
#include <stddef.h>

// ADC hardware channels
#define STARTER_VOLTAGE         0
#define FLYBACK_VOLTAGE         1
#define OUTPUT_CURRENT          2
#define TEMP                    10

#define OVER_CURRENT_ERROR_THRESHOLD   2  // Number of consecutive over currents needed to trigger an error

static bool current_check_enable = true;
static uint16_t current_average = 0;

static void starter_voltage_check(uint16_t voltage)
{
    if((keeper.common.current_state == STARTUP_STATE || keeper.common.current_state == ON_STATE)
       && voltage > KEEPER_OVER_VOLTAGE_COUNTS_STARTER){
        keeper.common.error_adc = voltage;
        keeper.common.error_code = KEEPER_OVER_VOLTAGE_STARTER_ERROR;
        keeper.common.current_state = keeper_error_handler();
    }
}

static void flyback_voltage_check(uint16_t voltage)
{
    if((keeper.common.current_state == STARTUP_STATE || keeper.common.current_state == ON_STATE)
       && voltage > KEEPER_MAX_V_OUT_COUNTS){
        keeper.common.error_adc = voltage;
        keeper.common.error_code = KEEPER_OVER_VOLTAGE_STARTUP_STATE_ERROR;
        keeper.common.current_state = keeper_error_handler();
    }
}

static void current_check(uint16_t current)
{
    current_average = (9*current_average + current)/10;

    if((keeper.common.current_state == STARTUP_STATE || keeper.common.current_state == ON_STATE)
       && (current_average > KEEPER_MAX_I_OUT_COUNTS) && current_check_enable){
        keeper.common.error_adc = current;
        keeper.common.error_code = KEEPER_OVER_CURRENT_ERROR;
        keeper.common.current_state = keeper_error_handler();
    }
}

void ka_over_current_enable(bool enable)
{
    current_check_enable = enable;
}

void keeper_adc_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    adc_channel_register(TEMP, NULL);
    adc_channel_register(STARTER_VOLTAGE, starter_voltage_check);
    adc_channel_register(FLYBACK_VOLTAGE, flyback_voltage_check);
    adc_channel_register(OUTPUT_CURRENT, current_check);
    
    adc_start();
}

void keeper_adc_deinit(void)
{
    adc_channel_deregister(TEMP);
    adc_channel_deregister(STARTER_VOLTAGE);
    adc_channel_deregister(FLYBACK_VOLTAGE);
    adc_channel_deregister(OUTPUT_CURRENT);
}

void ka_get_data(void)
{
    keeper.temperature = adc_get_value(RETURN_MEDIAN, TEMP);
    keeper.output_current = adc_get_value(RETURN_MEDIAN, OUTPUT_CURRENT);
    keeper.flyback_voltage = adc_get_value(RETURN_MEDIAN, FLYBACK_VOLTAGE);
    keeper.starter_voltage = adc_get_value(RETURN_MEDIAN, STARTER_VOLTAGE);
}
