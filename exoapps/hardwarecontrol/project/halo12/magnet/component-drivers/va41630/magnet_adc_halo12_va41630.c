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
#include "magnet_adc.h"
#include "magnet_mcu.h"

// ADC hardware channels
#define OUTPUT_CURRENT          0
#define OUTPUT_VOLTAGE          1
#define TEMP                    10

void magnet_adc_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    adc_channel_register(TEMP, NULL);
    adc_channel_register(OUTPUT_VOLTAGE, NULL);
    adc_channel_register(OUTPUT_CURRENT, NULL);
    
    adc_start();
}

void magnet_adc_deinit(void)
{
    adc_channel_deregister(TEMP);
    adc_channel_deregister(OUTPUT_VOLTAGE);
    adc_channel_deregister(OUTPUT_CURRENT);
}

void ma_get_data(void)
{
    magnet.temperature = adc_get_value(RETURN_MEDIAN, TEMP);
    magnet.output_current = adc_get_value(RETURN_MEDIAN, OUTPUT_CURRENT);
    magnet.output_voltage = adc_get_value(RETURN_MEDIAN, OUTPUT_VOLTAGE);
}
