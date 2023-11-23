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

#include <stdbool.h>
#include "timer/hal_timer.h"

#define ONE_MILLISECOND       1

static bool timer_flag = false;

static void kt_interrupt_callback(void)
{
    timer_flag = true;
}

void keeper_timer_init(void)
{
    timer_init_t keeper_control_timer;
    keeper_control_timer.channel = TIMER_THREE;
    keeper_control_timer.interrupt_callback = kt_interrupt_callback;
    keeper_control_timer.interrupt_priority = 0;
    keeper_control_timer.rst_value = ONE_MILLISECOND;
    keeper_control_timer.timer_units = TIMER_MILLI;
    timer_init(&keeper_control_timer);
    timer_enable_interrupts(TIMER_THREE, true);
    timer_start(TIMER_THREE, true);
}

void keeper_timer_deinit(void)
{
    timer_start(TIMER_THREE, false);
    timer_enable_interrupts(TIMER_THREE, false);
}

bool kt_timer_flag_check(void)
{
    return timer_flag;
}

void kt_timer_flag_reset(void)
{
    timer_flag = false;
}