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
#include "valve_hardware_port.h"

#define ONE_MILLISECOND       1
#define TEN_MILLISECONDS      10

static bool control_timer_flag = false;

static void vt_control_interrupt_callback(void)
{
    control_timer_flag = true;
}

static void vt_lv_interrupt_callback(void)
{
    timer_start(TIMER_FIVE, false);
    timer_enable_interrupts(TIMER_FIVE, false);
    vhp_lv_clear();
}

void valve_timer_init(void)
{
    // control timer init
    timer_init_t valve_control_timer;
    valve_control_timer.channel = TIMER_FOUR;
    valve_control_timer.interrupt_callback = vt_control_interrupt_callback;
    valve_control_timer.interrupt_priority = 0;
    valve_control_timer.rst_value = ONE_MILLISECOND;
    valve_control_timer.timer_units = TIMER_MILLI;
    timer_init(&valve_control_timer);
    timer_enable_interrupts(TIMER_FOUR, true);
    timer_start(TIMER_FOUR, true);

    // latch valve timer init
    timer_init_t latch_valve_timer;
    latch_valve_timer.channel = TIMER_FIVE;
    latch_valve_timer.interrupt_callback = vt_lv_interrupt_callback;
    latch_valve_timer.interrupt_priority = 0;
    latch_valve_timer.rst_value = TEN_MILLISECONDS;
    latch_valve_timer.timer_units = TIMER_MILLI;
    timer_init(&latch_valve_timer);
}

void valve_timer_deinit(void)
{
    timer_start(TIMER_FOUR, false);
    timer_enable_interrupts(TIMER_FOUR, false);
}

void vt_start(void)
{
    timer_set_t latch_valve_timer;
    latch_valve_timer.channel = TIMER_FIVE;
    latch_valve_timer.cnt_value = TEN_MILLISECONDS;
    latch_valve_timer.timer_units = TIMER_MILLI;
    timer_set(&latch_valve_timer);              // reset the timer to 10ms
    timer_enable_interrupts(TIMER_FIVE, true);
    timer_start(TIMER_FIVE, true);
}

bool vt_timer_flag_check(void)
{
    return control_timer_flag;
}

void vt_timer_flag_reset(void)
{
    control_timer_flag = false;
}