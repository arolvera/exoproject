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

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stddef.h>
#include "common_comp.h"

static cc_callback_t interrupt_callback_0;
static cc_callback_t interrupt_callback_1;
static cc_callback_t interrupt_callback_2;
static cc_callback_t interrupt_callback_3;

void common_comp_init(cc_callback_t cc_ic, uint8_t channel)
{
    switch(channel)
    {
        case 0:
            interrupt_callback_0 = cc_ic;
            break;
        case 1:
            interrupt_callback_1 = cc_ic;
            break;
        case 2:
            interrupt_callback_2 = cc_ic;
            break;
        case 3:
            interrupt_callback_3 = cc_ic;
            break;            
    }
}

ISR(ANACOMP0_vect)
{
    (*interrupt_callback_0)();
}

ISR(ANACOMP1_vect)
{
    (*interrupt_callback_1)();
}

ISR(ANACOMP2_vect)
{
    (*interrupt_callback_2)();
}

ISR(ANACOMP3_vect)
{
    (*interrupt_callback_3)();
}