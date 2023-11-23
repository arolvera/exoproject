/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "comp/common_comp.h"
#include "anode_control.h"
#include "anode_event_handlers.h"

#define COMPARATOR_CHANNEL  3

static void ac_interrupt_callback(void)
{
    common.current_state = anode_spark_detected_handler();
}

void ac_spark_detect_enable(void)
{
    ACSR |= (1 << AC3IF);   // clear flag
    AC3CON |= (1 << AC3IE); // enable interrupts
}

void ac_spark_detect_disable(void)
{
    AC3CON &= ~(1 << AC3IE);
}

void anode_comp_init(void)
{
    uint8_t ac3con = 0;
    ac3con |= (1 << AC3EN); // enable comparator
    ac3con |= (1 << AC3M1); // Vref/2.13
    AC3CON = ac3con;
    
    common_comp_init(ac_interrupt_callback, COMPARATOR_CHANNEL);
}
