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
#include "anode_adc.h"
#include "anode_event_handlers.h"
#include "anode_mcu.h"

// ADC hardware channels
#define OUTPUT_VOLTAGE          2
#define Y_VOLTAGE               3
#define OUTPUT_CURRENT          4
#define RAW_INPUT_VOLTAGE       5
#define FILTERED_INPUT_VOLTAGE  6
#define TEMP                    10

#define OVER_CURRENT_ERROR_THRESHOLD   1  // Number of consecutive over currents needed to trigger an error

static bool current_check_enable = true;
static uint8_t over_current_counter = 0;

static void x_voltage_check(uint16_t voltage)
{
    uint16_t x_voltage = 0;
    if(voltage > anode.y_voltage){
        x_voltage = voltage - anode.y_voltage;
    }
    if((anode.common.current_state == STARTUP_STATE || anode.common.current_state == ON_STATE)
       && ((x_voltage > (ANODE_OVER_VOLTAGE_COUNTS / 2)) || (voltage > ANODE_OVER_VOLTAGE_COUNTS))){
        anode.common.error_adc = voltage;
        anode.common.error_code = ANODE_OVER_VOLTAGE_ERROR;
        anode.common.current_state = anode_error_handler();
    }
}

static void y_voltage_check(uint16_t voltage)
{
    if((anode.common.current_state == STARTUP_STATE || anode.common.current_state == ON_STATE)
       && voltage > (ANODE_OVER_VOLTAGE_COUNTS / 2)){
        anode.common.error_adc = voltage;
        anode.common.error_code = ANODE_OVER_VOLTAGE_ERROR;
        anode.common.current_state = anode_error_handler();
    }
}

static void input_voltage_check(uint16_t voltage)
{
    if((anode.common.current_state == STARTUP_STATE || anode.common.current_state == ON_STATE)
       && voltage < ANODE_MIN_INPUT_VOLTAGE_COUNTS){
        anode.common.error_adc = voltage;
        anode.common.error_code = ANODE_INPUT_VOLTAGE_LOW_ERROR;
        anode.common.current_state = anode_error_handler();
    }
}

static void current_check(uint16_t current)
{
    if((anode.common.current_state == STARTUP_STATE || anode.common.current_state == ON_STATE)
       && (current > ANODE_OVER_CURRENT_COUNTS) && current_check_enable){
        over_current_counter++;
        if(over_current_counter > OVER_CURRENT_ERROR_THRESHOLD)
        {
            anode.common.error_adc = current;
            anode.common.error_code = ANODE_OVER_CURRENT_ERROR;
            anode.common.current_state = anode_error_handler();
        }
    } else over_current_counter = 0;
}

void aa_over_current_enable(bool enable)
{
    current_check_enable = enable;
}

void anode_adc_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    adc_channel_register(RAW_INPUT_VOLTAGE, input_voltage_check);
    adc_channel_register(FILTERED_INPUT_VOLTAGE, NULL);
    adc_channel_register(TEMP, NULL);
    adc_channel_register(OUTPUT_VOLTAGE, x_voltage_check);
    adc_channel_register(Y_VOLTAGE, y_voltage_check);
    adc_channel_register(OUTPUT_CURRENT, current_check);

    adc_start();
}

void anode_adc_deinit(void)
{
    adc_channel_deregister(RAW_INPUT_VOLTAGE);
    adc_channel_deregister(FILTERED_INPUT_VOLTAGE);
    adc_channel_deregister(TEMP);
    adc_channel_deregister(OUTPUT_VOLTAGE);
    adc_channel_deregister(Y_VOLTAGE);
    adc_channel_deregister(OUTPUT_CURRENT);
}

void aa_get_data(void)
{
    anode.raw_input_voltage = adc_get_value(RETURN_MEDIAN, RAW_INPUT_VOLTAGE);
    anode.filtered_input_voltage = adc_get_value(RETURN_MEDIAN, FILTERED_INPUT_VOLTAGE);
    anode.temperature = adc_get_value(RETURN_MEDIAN, TEMP);
    anode.output_current = adc_get_value(RETURN_MEDIAN, OUTPUT_CURRENT);
    anode.y_voltage = adc_get_value(RETURN_MEDIAN, Y_VOLTAGE);
    anode.output_voltage = adc_get_value(RETURN_MEDIAN, OUTPUT_VOLTAGE);
    anode.x_voltage = 0;
    if(anode.output_voltage > anode.y_voltage){
        anode.x_voltage = anode.output_voltage - anode.y_voltage;
    }
}
