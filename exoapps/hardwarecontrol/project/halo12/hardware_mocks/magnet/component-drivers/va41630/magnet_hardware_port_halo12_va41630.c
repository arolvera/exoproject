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

#include "hardware_port/hal_hardware_port.h"
#include "magnet_event_handlers.h"
#include "magnet_hardware_port.h"
#include "magnet_mcu.h"

#define OVER_CURRENT       2 // PORTA[2]

static void over_current_callback(void)
{
    magnet.common.current_state = magnet_error_handler();
}

void magnet_hardware_port_init(void)
{

}
