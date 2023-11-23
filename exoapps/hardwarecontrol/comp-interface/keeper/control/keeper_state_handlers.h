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

#ifndef KEEPER_STATE_HANDLERS_H
#define KEEPER_STATE_HANDLERS_H

void ksh_set_counters(uint16_t lct);

// State handlers
void keeper_startup_state(void) __attribute__((weak));
void keeper_on_state(void) __attribute__((weak));
void keeper_error_state(void) __attribute__((weak));
void keeper_spark_detected(void) __attribute__((weak));
void keeper_starter_enable(bool enable) __attribute__((weak));

#endif	/* KEEPER_STATE_HANDLERS_H */

