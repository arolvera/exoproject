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

#include "timer/hal_timer.h"
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stddef.h>
#include <signal.h>
#include <stdio.h>
#include "sys/sys_timers.h"

// add more timer callbacks and ISRs as needed
static ct_callback_t interrupt_callback_0;
static ct_callback_t interrupt_callback_1;
static ct_callback_t interrupt_callback_2;
static ct_callback_t interrupt_callback_3;
static ct_callback_t interrupt_callback_4;
static ct_callback_t interrupt_callback_5;

timer_t timerid;
struct  sigevent   sev;
struct  sigaction  sa;
struct  itimerspec its;
static int first_init = 1;
static int timer_index;
static timer_ops_t tops[] = {{0}, {0}, {0}, {0}, {0}, {0}};

#define MICRO_SCALE  (50) /* scale value for microseconds at CPU clk = 100MHz */
#define MILLI_SCALE  (MICRO_SCALE * 1000) /* scale value for milliseconds at CPU clk = 100MHz */


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


void timer_init(timer_init_t *tmr_init)
{
    if(tmr_init != 0) {
        /* If first init, use linux real time timer.  Else use software timers based off
         *  real time timer (similar to FreeRTOS software timers based of
         *  systick for cortex M parts) */
        if (first_init) {
            if(tmr_init->timer_units == TIMER_MICRO) {
                its.it_value.tv_nsec = tmr_init->rst_value * 1000;
                its.it_interval.tv_nsec = tmr_init->rst_value * 1000;
            } else if(tmr_init->timer_units == TIMER_MILLI){
                its.it_value.tv_nsec = tmr_init->rst_value * 1000000;
                its.it_interval.tv_nsec = tmr_init->rst_value * 1000000;
            }

            first_init = 0;
            sa.sa_flags = SA_SIGINFO;
            sa.sa_sigaction = tmr_init->interrupt_callback;
            sigemptyset(&sa.sa_mask);

            sev.sigev_notify = SIGEV_SIGNAL;
            sev.sigev_signo = SIGRTMIN;
            sigaction(SIGRTMIN, &sa, NULL);

            timer_create(CLOCK_REALTIME, &sev, &timerid);
            timer_settime(timerid, 0, &its, NULL);
        }
        tops[timer_index].cb = tmr_init->interrupt_callback;
        tops[timer_index].timer_cnts = tmr_init->rst_value;
        tops[timer_index].rst_val = tmr_init->rst_value;
        sys_timer_w_cb_start(&tops[timer_index]);
        timer_index++;
    }
}

void timer_start(uint8_t channel, bool enable)
{

}

void timer_enable_interrupts(uint8_t channel, bool enable)
{

}

void timer_set(timer_set_t *timer_set)
{

}

uint32_t timer_value_get(uint8_t channel)
{
    return 0;
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

