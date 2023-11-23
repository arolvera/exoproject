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

#include "anode/comp-driver/anode_timer.h"
#include "timer/hal_timer.h"
#include <avr/io.h>
#include <stdbool.h>
#include <stddef.h>

#define ONE_MILLISECOND   0x08 // 1ms flag
#define FIVE_HUNDRED_US   0x04 // 500us flag

static bool timer_flag = false;

static void at_interrupt_callback(void)
{
    timer_flag = true;
}

void anode_timer_init(void)
{
    uint16_t tccr1b = 0;
    tccr1b |= ((1 << CS10) | (1 << CS12)); // divide clock by 1024
    tccr1b |= (1 << WGM12);                // clear timer on match
    TCCR1B = tccr1b;
    
    OCR1A = FIVE_HUNDRED_US;               // set a flag every 500us
    
    common_timer_init(at_interrupt_callback);
    
    TIMSK1 |= (1 << OCIE1A);               // enable interrupts
}

bool at_timer_flag_check(void)
{
    return timer_flag;
}

void at_timer_flag_reset(void)
{
    timer_flag = false;
}