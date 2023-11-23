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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <stdbool.h>
#include "adc/hal_adc.h"

void adc_start(uint8_t first_channel)
{
    uint8_t adcsra = 0;
    adcsra |= (1 << ADEN);   // enable ADC
    adcsra |= (1 << ADIE);   // enable interrupts
    adcsra |= (1 << ADPS2);  // divide clock by 64
    adcsra |= (1 << ADPS1);
    ADCSRA = adcsra;
    
    uint8_t adcsrb = 0;
    adcsra |= (1 << AREFEN); // connect reference voltage to external pin
    ADCSRB = adcsrb;
    
    uint8_t admux = 0;
    admux |= (1 << REFS1);  // internal reference voltage
    admux |= (1 << REFS0);
    admux |= first_channel; // start at the first channel
    ADMUX = admux;

    ADCSRA |= (1 << ADSC);  // let 'er rip
 }

ISR(ADC_vect)
{
    uint8_t new_channel = adc_interrupt_callback(ADCL | (ADCH << 8));// call the generic callback

    ADMUX &= ~0x1F;            // clear the mux
    ADMUX |= new_channel; // set it to the new channel
    ADCSRA |= (1 << ADSC);     // let 'er rip
}
