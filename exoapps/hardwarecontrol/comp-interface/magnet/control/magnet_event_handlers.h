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

#include "component_service.h"

#ifndef MAGNET_EVENT_HANDLERS_H
#define	MAGNET_EVENT_HANDLERS_H

// Event handlers
states_t magnet_on_command_handler(void) __attribute__((weak));
states_t magnet_power_good_handler(void) __attribute__((weak));
states_t magnet_off_command_handler(void) __attribute__((weak));
states_t magnet_error_handler(void) __attribute__((weak));
states_t magnet_error_cleared_handler(void) __attribute__((weak));

#endif	/* MAGNET_EVENT_HANDLERS_H */

