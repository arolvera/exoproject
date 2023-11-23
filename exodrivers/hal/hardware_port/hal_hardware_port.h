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

#ifndef HAL_HARDWARE_PORT_H
#define	HAL_HARDWARE_PORT_H

#include <stdint.h>

typedef void (*hp_callback_t)(void);

void hardware_port_cb_register(hp_callback_t hp_ic, uint8_t port);

#endif	/* HAL_HARDWARE_PORT_H */