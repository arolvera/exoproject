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

#include "co_core.h"                        // CAN Open Core Functions
#include "driver/co_timer_VA41630.h"   // Header for this file
#include "timer/hal_timer.h"

/******************************************************************************
* PRIVATE DEFINES
******************************************************************************/

/* Current timer counter value */
int timer_counter = 0;

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/

static void DrvTimerInit(uint32_t freq);
static void DrvTimerStart(void);
static uint8_t DrvTimerUpdate(void);
static uint32_t DrvTimerDelay(void);
static void DrvTimerReload(uint32_t reload);
static void DrvTimerStop(void);

/******************************************************************************
* PUBLIC VARIABLE
******************************************************************************/
/* get external CANopen node */
extern CO_NODE tc_node;

/* Pointers to driver functions */
const CO_IF_TIMER_DRV ATSAMV71TimerDriver = {
    DrvTimerInit,
    DrvTimerReload,
    DrvTimerDelay,
    DrvTimerStop,
    DrvTimerStart,
    DrvTimerUpdate
};

/******************************************************************************
* PRIVATE FUNCTIONS
******************************************************************************/
#define CO_TIMER_NUM 7

/* This function is called after period expires */
static void timer_cb(void)
{
    timer_counter--;
    COTmrService(&tc_node.Tmr);
}

static void DrvTimerInit(uint32_t freq)
{
    /* Register callback function for CH0 period interrupt */
    timer_init_t tinit = {
        .channel = CO_TIMER_NUM,
        .rst_value = 100000,
        .interrupt_priority = 4,
        .interrupt_callback = &timer_cb
    };
    timer_init(&tinit);
    timer_cb_register(&timer_cb, CO_TIMER_NUM);
}

static void DrvTimerStart(void)
{
    /* Start the timer channel 0*/
    timer_start(CO_TIMER_NUM, true);
}

static uint8_t DrvTimerUpdate(void)
{
    /* return 1 if timer event is elapsed, otherwise 0 */
    return timer_value_get(CO_TIMER_NUM) == 0 ? 1 : 0;
}

static uint32_t DrvTimerDelay(void)
{
    /* return current timer counter value */
    return timer_value_get(CO_TIMER_NUM);
}

static void DrvTimerReload(uint32_t reload)
{
    /* reload timer counter value with given reload value */
    timer_counter = reload;
}

static void DrvTimerStop(void)
{
    /* stop timer and clear counter value */
    timer_counter = 0;
    timer_start(CO_TIMER_NUM, false);
}
