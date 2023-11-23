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
#include "common_comp.h"
#include "keeper_comp.h"
#include "keeper_control.h"
#include "keeper_event_handlers.h"

#define COMPARATOR_CHANNEL  2

static void kc_interrupt_callback(void)
{
    common.current_state = keeper_spark_detected_handler();
}

void kc_spark_detect_enable(void)
{
    ACSR |= (1 << AC2IF);   // clear flag
    AC2CON |= (1 << AC2IE); // enable interrupts
}

void kc_spark_detect_disable(void)
{
    AC2CON &= ~(1 << AC2IE);
}

void keeper_comp_init(void)
{
    uint8_t ac2con = 0;
    ac2con |= (1 << AC2EN); // enable comparator
    ac2con |= (1 << AC2M1); // Vref/2.13
    AC2CON = ac2con;
    
    common_comp_init(kc_interrupt_callback, COMPARATOR_CHANNEL);
}
