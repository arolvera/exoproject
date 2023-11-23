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
#include "keeper_adc.h"
#include "keeper_control.h"
#include <stddef.h>

// ADC hardware channels
#define OUTPUT_VOLTAGE          8
#define OUTPUT_CURRENT          3
#define INPUT_VOLTAGE           10

#define ZERO_CURRENT_OFFSET     100 // Counts

void keeper_ADC_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    ca_channel_register(INPUT_VOLTAGE, NULL);
    ca_channel_register(OUTPUT_VOLTAGE, NULL);
    ca_channel_register(OUTPUT_CURRENT, NULL);
    
    common_ADC_init();
}

void ka_get_data(void)
{
    uint16_t current = ca_get_value(RETURN_MEDIAN, OUTPUT_CURRENT);
    if(ZERO_CURRENT_OFFSET > current)
    {
        current = 0;
    } else {
        current -= ZERO_CURRENT_OFFSET;
    }
    keeper.output_current = current;
    keeper.output_voltage = ca_get_value(RETURN_MEDIAN, OUTPUT_VOLTAGE);
    keeper.input_voltage = ca_get_value(RETURN_MEDIAN, INPUT_VOLTAGE);
}
