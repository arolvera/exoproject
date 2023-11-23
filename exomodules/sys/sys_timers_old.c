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
#include <stdint.h>
#include "sys_timers_old.h"
#include "definitions.h"              /* Picked up FreeRTOS definition */
#include "utilities/trace/trace.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#ifdef FREERTOS_H
#define sys_timer_yeild() taskYIELD();  /* Yeild to other tasks */
#else
#define sys_timer_yeild() ;             /* Just spin            */
#endif

volatile uint32_t *timer_list[MAX_SYS_TIMERS]  = {NULL};


static volatile int delayed_ticks = 0;
static OSAL_MUTEX_HANDLE_TYPE sys_timer_lock;

/**
 * ISR Callback for when the timer list is being manipulated.  Just track the 
 * number of ticks until the timer countdown ISR is re-enabled.
 * @param status see harmony driver
 * @param context see harmony driver
 */
static void sys_timer_count_ticks(TC_TIMER_STATUS status, uintptr_t context)
{
    delayed_ticks++;
    trace_timer++;
}

/**
 * Timer 0 channel 1 ISR callback function. The system software timers are
 * serviced here.
 * @param status see harmony driver
 * @param context see harmony driver
 */
static void sys_timer_ch1_callback(TC_TIMER_STATUS status, uintptr_t context)
{
    int ticks = delayed_ticks + 1;
    trace_timer++;
    // update the 8 system software timers
    for (int i = 0; i < MAX_SYS_TIMERS; i++) {
        SYSTMR *p = timer_list[i];
        if(p && *p > 0) {           /* Not a NULL pointer & timer > 0 */
            SYSTMR remaining = *p;
            if(ticks > remaining) {
                *p = 0;             /* Do not subtract more than is left */
            } else {
                *p -= ticks;
            }
            if(*p == 0) {
                /* Expired - remove from list */
                timer_list[i] = NULL;
            }
        }
    }
    delayed_ticks = 0;
}

/**
 * Switch the ISR to just count ticks while we manipulate memory
 */
static void sys_timeres_tick_delay(void)
{
    TC0_CH1_TimerCallbackRegister(sys_timer_count_ticks, (uintptr_t) NULL);
}

/**
 * Restore regular timer processing
 */
static void sys_timeres_tick_restore(void)
{
    TC0_CH1_TimerCallbackRegister(sys_timer_ch1_callback, (uintptr_t) NULL);
}

/**
 * Lock a mutex to protect multiple tasks from interfering with each other when
 * they manipulate the list, and move the ISR to just count ticks while the
 * tasks are doing the same.
 */
static void sys_timers_lock(void)
{
    OSAL_MUTEX_Lock(&sys_timer_lock, OSAL_WAIT_FOREVER);
    sys_timeres_tick_delay();
}

/**
 * Unlock mutex and resume normal ISR
 */
static void sys_timers_unlock(void)
{
    sys_timeres_tick_restore();
    OSAL_MUTEX_Unlock(&sys_timer_lock);
}

/*
// Performs the necessary steps for timer 0 channel 1 to operate in the system
// 
// 
// Receives:    void
// 
// Returns:     void
// 
*/
void sys_timer_init(void)
{
    OSAL_MUTEX_Create(&sys_timer_lock, NULL);
    // register the callback function and start the timer
    TC0_CH1_TimerCallbackRegister(sys_timer_ch1_callback, (uintptr_t) NULL);
    TC0_CH1_TimerStart();
}

/*
// Start a system timer for the calling function. Return Success or Failure
// status. Failure to start a timer is possible if all 8 system timers are
// in use.
//
//
// Receives: tval - the time value of the timer to start
//           tmr  - a pointer to the timer count the calling function uses
//
// Returns:   Success - timer number used (0 - 7)
//            Failure - NO_TIMER (defined as 0xff)
*/
int sys_timer_start(uint32_t tval, volatile SYSTMR *tmr)
{
    int i = 0;
    SYSTMR *p = NULL;
    
    sys_timers_lock();

    
    TraceDbg(TrcMsgDbg, "tval:%d tmr:%p", tval, tmr, 0,0,0,0);
    for (i = 0; i < MAX_SYS_TIMERS && p == NULL; i++) {
        SYSTMR *used = timer_list[i];
        TraceDbg(TrcMsgDbg, "timer[%d]:%p", i, used, 0,0,0,0);
        if(used == NULL) { /* Not used currently */
            p = used;
            *tmr = tval;
            timer_list[i] = tmr;
            p = tmr;
            TraceDbg(TrcMsgDbg, "tmr:%p:%d tval:%d", tmr, *tmr, tval,0,0,0);
        }
    }
    if (p == NULL)  {
        i = NO_TIMER;
    }
    TraceDbg(TrcMsgDbg, "err:%d timer:%p val:%d", i, p, *tmr,0,0,0);
    sys_timers_unlock();
    
    return i; // timer started, return the timer number
}


/*
// Restart a system timer for the calling function. If the timer to restart
// has lapsed it was also released and is no longer valid. In this case
// NO_TIMER is returned.
//
//
// Receives: tval - the time value of the timer to restart
//           tmr  - a pointer to the timer count the calling function uses
//
// Returns:   Success = timer number used (0 - 7)
//            Failure = NO_TIMER (defined as 0xff)
*/
int sys_timer_restart(uint32_t tval, volatile SYSTMR *tmr)
{
    int i = 0;
    SYSTMR *p = NULL;
    
    sys_timers_lock();
    
    // find the timer to restart
    for (i = 0; i < MAX_SYS_TIMERS && p == NULL; i++) {
        p = timer_list[i];
        if(p == tmr) {
            *p = tval;
        } else {
            p = NULL;
        }
    }
    // if the timer to restart was no longer valid (lapsed)
    // return "no timer found"
    if (i > MAX_SYS_TIMERS) {
        i = NO_TIMER;
    }
    
    sys_timers_unlock();
        
    return i;
}


/*
// Prematurely ends a timer and releases it back to the system for use.
//
//
// Receives: tmr - timer to be aborted
//
// Returns:  void
*/
void sys_timer_abort(volatile SYSTMR *tmr)
{
    int i = 0;
    SYSTMR *p = NULL;
    
    sys_timers_lock();

    // find the timer to be released and release it
    for (i = 0; i < MAX_SYS_TIMERS && p == NULL; i++) {
        p = timer_list[i];
        if(p == tmr) {
            /* zero timer and take it out of the list */
            *p = 0;
            timer_list[i] = NULL;
        } else {
            p = NULL;
        }
    }
    sys_timers_unlock();
}


/*
// starts a timer for the time indicated by 'td'. waits on the timer to
// lapse then returns.
//
//
// Receives:  td - timer time value * 100us
//
// Returns:   void
*/
int sys_timer_delay(uint32_t td)
{
    int err = 0;
    SYSTMR tmr;
    
    err = sys_timer_start(td, &tmr);
    if(err == NO_TIMER) {
        err = -1;
    } else {
        err = 0;
        while (tmr > 0) {
            sys_timer_yeild();
        }
    }
    return err;
}
