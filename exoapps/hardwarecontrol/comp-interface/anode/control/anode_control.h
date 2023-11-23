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

#ifndef ANODE_CONTROL_H
#define ANODE_CONTROL_H

#include "component_service.h"
#include "msg_callback.h"

int ac_sync(message_t *msg) __attribute__((weak));
void anode_control_init(void) __attribute__((weak));
states_t anode_state_get(void) __attribute__((weak));

#endif	/* ANODE_CONTROL_H */

