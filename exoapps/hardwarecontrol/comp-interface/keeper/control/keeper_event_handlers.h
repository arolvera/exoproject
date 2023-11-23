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


#ifndef KEEPER_EVENT_HANDLERS_H
#define KEEPER_EVENT_HANDLERS_H
#include "component_service.h"

// Event handlers
states_t keeper_on_command_handler(void) __attribute__((weak));
states_t keeper_spark_detected_handler(void) __attribute__((weak));
states_t keeper_stable_handler(void) __attribute__((weak));
states_t keeper_restart_handler(void) __attribute__((weak));
states_t keeper_off_command_handler(void) __attribute__((weak));
states_t keeper_error_handler(void) __attribute__((weak));
states_t keeper_error_cleared_handler(void) __attribute__((weak));

#endif	/* KEEPER_EVENT_HANDLERS_H */

