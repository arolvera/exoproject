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

#ifndef ANODE_STATE_HANDLERS_H
#define ANODE_STATE_HANDLERS_H

void ash_reset_counters() __attribute__((weak));

// State handlers
void anode_startup_state(void) __attribute__((weak));
void anode_on_state(void) __attribute__((weak));
void anode_error_state(void) __attribute__((weak));

#endif	/* ANODE_STATE_HANDLERS_H */

