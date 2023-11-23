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
#include "anode_hardware_port.h"

void ahp_buck_enable(void)
{
    PORTC &= ~0x01;
}

void ahp_boost_enable(void)
{
    PORTB &= ~0x40;
}

void ahp_disable(void)
{
    PORTB |= 0x40;
    PORTC |= 0x01;
}

void anode_hardware_port_init(void)
{
    DDRC = 0x80;          // Set DAC output pin to output
    PORTC &= ~(0x80);     // Clear output pin low
    DDRB = 0x40;          // Set boost enable to output
    DDRC = 0x01;          // Set buck enable to output
    ahp_disable();
}