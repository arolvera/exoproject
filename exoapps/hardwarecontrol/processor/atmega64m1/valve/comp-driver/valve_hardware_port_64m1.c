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

#include <avr/io.h>
#include <stddef.h>
#include "valve_hardware_port.h"
#include "valve_timer.h"

void vhp_lv_clear(void)
{
    PORTB &= ~0x82;
}

void vhp_lv_open(void)
{
    PORTB |= 0x02;
    vt_start();
}

void vhp_lv_close(void)
{
    PORTB |= 0x80;
    vt_start();
}

void valve_hardware_port_init(void)
{
    DDRB = 0x82;          // Set LV pins to outputs
    vhp_lv_clear();       // Set LV outputs low
}
