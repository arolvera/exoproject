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

#ifndef HAL_ADC_H
#define	HAL_ADC_H

#include <stdint.h>
#include <stdbool.h>

typedef void (*data_ready_func_t)(uint16_t value);

typedef enum 
{
    RETURN_INST,    // Return instantaneous sample value
    RETURN_MEDIAN,  // Return sample data array median
} adc_return_t;

bool adc_data_ready_check(void);
void adc_data_ready_reset(void);
uint16_t adc_get_value(adc_return_t data_type, uint8_t channel);
void adc_channel_register(uint8_t adc_chan, data_ready_func_t cb);
void adc_channel_deregister(uint8_t adc_chan);
void adc_start(void);
void adc_init(void);
void adc_enable(uint8_t first_channel);
uint8_t adc_interrupt_callback(uint16_t value);

#endif	/* HAL_ADC_H */

