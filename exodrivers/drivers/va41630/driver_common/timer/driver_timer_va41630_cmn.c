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

#include "device.h"
#include "timer/hal_timer.h"
#include <stdbool.h>
#include <stddef.h>

// add more timer callbacks and ISRs as needed
static ct_callback_t interrupt_callback_0;
static ct_callback_t interrupt_callback_1;
static ct_callback_t interrupt_callback_2;
static ct_callback_t interrupt_callback_3;
static ct_callback_t interrupt_callback_4;
static ct_callback_t interrupt_callback_5;

#define MICRO_SCALE  (50) /* scale value for microseconds at CPU clk = 100MHz */
#define MILLI_SCALE  (MICRO_SCALE * 1000) /* scale value for milliseconds at CPU clk = 100MHz */

static VOR_TIM_Type *timers = VOR_TIM0;

void timer_init(timer_init_t *tmr_init)
{
    if(tmr_init->channel < NUMBER_OF_TIMERS) {
        VOR_TIM_Type *timer;

        timer = &timers[tmr_init->channel];

        VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << tmr_init->channel); // enable the clock

        uint32_t scaled_time = (tmr_init->timer_units == TIMER_MILLI) ? MILLI_SCALE : MICRO_SCALE;
        timer->RST_VALUE = tmr_init->rst_value * scaled_time;

        NVIC_EnableIRQ(TIM0_IRQn + tmr_init->channel); // enable the timer NVIC entry
        NVIC_SetPriority(TIM0_IRQn + tmr_init->channel, tmr_init->interrupt_priority); // set the interrupt priority

        timer_cb_register(tmr_init->interrupt_callback, tmr_init->channel); // register the callback
    }
}

void timer_deinit(timer_init_t *tmr_init)
{
    if(tmr_init->channel < NUMBER_OF_TIMERS) {
        VOR_SYSCONFIG->TIM_CLK_ENABLE &= ~(1 << tmr_init->channel); // enable the clock
        NVIC_DisableIRQ(TIM0_IRQn + tmr_init->channel); //
    }
}

void timer_start(uint8_t channel, bool enable)
{
    if(channel < NUMBER_OF_TIMERS) {
        if(enable == true) {
            timers[channel].ENABLE |= (1 << TIM_CTRL_ENABLE_Pos);// enable timer
        } else {
            timers[channel].ENABLE &= ~(1 << TIM_CTRL_ENABLE_Pos);// disable timer
        }
    }
}

void timer_enable_interrupts(uint8_t channel, bool enable)
{
    if(channel < NUMBER_OF_TIMERS) {
        if(enable == true) {
            timers[channel].CTRL |= (1 << TIM_CTRL_IRQ_ENB_Pos);// enable interrupts
        } else {
            timers[channel].CTRL &= ~(1 << TIM_CTRL_IRQ_ENB_Pos);// disable interrupts
        }
    }
}

void timer_set(timer_set_t *timer_set)
{
    if(timer_set->channel < NUMBER_OF_TIMERS) {
        uint32_t scaled_time = (timer_set->timer_units == TIMER_MILLI) ? MILLI_SCALE : MICRO_SCALE;
        timers[timer_set->channel].CNT_VALUE = timer_set->cnt_value * scaled_time;
    }
}

uint32_t timer_value_get(uint8_t channel)
{
    return timers[channel].CNT_VALUE;
}

void timer_cb_register(ct_callback_t ct_ic, uint8_t channel)
{
    // add more callbacks and cases as needed
    switch(channel)
    {
        case 0:
            interrupt_callback_0 = ct_ic;
            break;
        case 1:
            interrupt_callback_1 = ct_ic;
            break;
        case 2:
            interrupt_callback_2 = ct_ic;
            break;
        case 3:
            interrupt_callback_3 = ct_ic;
            break;
        case 4:
            interrupt_callback_4 = ct_ic;
            break;
        case 5:
            interrupt_callback_5 = ct_ic;
            break;
    }
}

void TIM0_IRQHandler(void)
{
    if(interrupt_callback_0 != NULL){
        (*interrupt_callback_0)();
    }
}

void TIM1_IRQHandler(void)
{
    if(interrupt_callback_1 != NULL){
        (*interrupt_callback_1)();
    }
}

void TIM2_IRQHandler(void)
{
    if(interrupt_callback_2 != NULL) {
        (*interrupt_callback_2)();
    }
}

void TIM3_IRQHandler(void)
{
    if(interrupt_callback_3 != NULL){
        (*interrupt_callback_3)();
    }
}

void TIM4_IRQHandler(void)
{
    if(interrupt_callback_4 != NULL){
        (*interrupt_callback_4)();
    }
}

void TIM5_IRQHandler(void)
{
    if(interrupt_callback_5 != NULL){
        (*interrupt_callback_5)();
    }
}