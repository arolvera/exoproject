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
#ifndef SYS_TIMERS_H
#define SYS_TIMERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ext_decl_define.h"

#define MAX_SYS_TIMERS  7
#define NO_TIMER        0xff

// time constants for use with the system software timers
//
// usage example for a timer of 15 milliseconds:
//
// SYSTMR timer_status;   // declare a timer status var
//
// StartTimer(15 MILLISECONDS, &timer_status);
//
// 'timer_status' always equals the time remaining in 100us units
// The started timer has lapsed when 'timer_status' = 0
//
#define MINUTES           * 600000
#define SECONDS           *  10000
#define MILLISECONDS      *     10
#define TENTHMILLIES      *      1

typedef uint32_t volatile SYSTMR;

typedef void* (*tmr_callback_t)(void*);
typedef struct timer_ops{
    int rst_val;
    SYSTMR timer_cnts;
    tmr_callback_t cb;
}timer_ops_t;

void sys_timer_w_cb_init(void);
int sys_timer_w_cb_start(volatile timer_ops_t *tmr);
int sys_timer_start(uint32_t, volatile SYSTMR *);
int sys_timer_restart(uint32_t, volatile SYSTMR *);
int sys_timer_delay(uint32_t);

void sys_timer_abort(volatile SYSTMR *);
void sys_timer_init(void);

#ifdef __cplusplus
}
#endif

#endif			// SYS_TIMERS_H
