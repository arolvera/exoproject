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
#include <stddef.h>
#include "common_timer.h"
#include "valve_hardware_port.h"
#include "valve_timer.h"

#define TEN_MILLISECONDS   0x50 // 10ms flag

static void vt_interrupt_callback(void)
{
    TIMSK1 &= ~(1 << OCIE1A);  // disable interrupts
    vhp_lv_clear();
}

void valve_timer_init(void)
{
    uint16_t tccr1b = 0;
    tccr1b |= ((1 << CS10) | (1 << CS12)); // divide clock by 1024
    tccr1b |= (1 << WGM12);                // clear timer on match
    TCCR1B = tccr1b;
    
    OCR1A = TEN_MILLISECONDS;              // set a flag every 10ms
    
    common_timer_init(vt_interrupt_callback);
}

void vt_start(void)
{
    TCNT1 = 0;                // reset the timer
    TIFR1 = (1 << OCF1A);     // clear the flag
    TIMSK1 |= (1 << OCIE1A);  // enable interrupts
}
