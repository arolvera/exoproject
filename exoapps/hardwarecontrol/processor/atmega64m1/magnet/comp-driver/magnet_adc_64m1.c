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
#include "magnet_adc.h"
#include "magnet_control.h"
#include <stddef.h>

// ADC hardware channels
#define INNER_CURRENT     10 
#define INNER_VOLTAGE     5
#define OUTER_CURRENT     2
#define OUTER_VOLTAGE     6
#define TEMPERATURE       3

void magnet_ADC_init(void)
{
    // The order you register the channels is also the scan order, so do the important ones last
    ca_channel_register(TEMPERATURE, NULL);
    ca_channel_register(INNER_VOLTAGE, NULL);
    ca_channel_register(OUTER_VOLTAGE, NULL);
    ca_channel_register(INNER_CURRENT, NULL);
    ca_channel_register(OUTER_CURRENT, NULL);
    
    common_ADC_init();
}

void ma_get_data(void)
{
    magnet.inner_current = ca_get_value(RETURN_MEDIAN, INNER_CURRENT);
    magnet.inner_voltage = ca_get_value(RETURN_MEDIAN, INNER_VOLTAGE);
    magnet.outer_current = ca_get_value(RETURN_MEDIAN, OUTER_CURRENT);
    magnet.outer_voltage = ca_get_value(RETURN_MEDIAN, OUTER_VOLTAGE);
    magnet.temperature   = ca_get_value(RETURN_MEDIAN, TEMPERATURE);
}
