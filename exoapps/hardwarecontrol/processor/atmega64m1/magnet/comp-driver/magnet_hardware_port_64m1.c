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
#include "magnet_hardware_port.h"

void mhp_enable(void)
{
    PORTB |= 0x80;
}

void mhp_disable(void)
{
    PORTB &= ~0x80;
}

void mhp_LED_on(void)
{
    PORTD |= 0x01;
}

void mhp_LED_off(void)
{
    PORTD &= ~0x01;
}

void magnet_hardware_port_init(void)
{
    DDRD = 0x01;          // Set LED pin to output
    mhp_LED_off();
    DDRB = 0x80;          // Set enable pin to output
    mhp_disable();
}
