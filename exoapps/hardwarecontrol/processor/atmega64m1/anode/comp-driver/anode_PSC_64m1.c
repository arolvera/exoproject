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
#include "anode_PSC.h"

void anode_PSC_init(void)
{
    uint8_t pllcsr = 0;
    pllcsr |= (1 << PLLF); // 64MHz clock
    pllcsr |= (1 << PLLE); // enable
    PLLCSR = pllcsr;
    
    uint8_t poc = 0;
    poc |= (1 << POEN0A); // enable 0A
    POC = poc;
    
    uint8_t pcnf = 0;
    pcnf |= (1 << POPA); // A active high
    pcnf &= (1 << POPB);
    PCNF = pcnf;
    
    PCTL |= (1 << PCLKSEL); // fast clock
    
    POCR0RA = 0x0FFF;
    POCR0SA = 0;
    POCR_RB = ANODE_MAX_PWM; // 40kHz PWM
    
    PMIC0 |= (1 << POVEN0); // disable overlap protection
    
    PCTL |= (1 << PRUN); // run PSC
}

void ap_set_PWM(uint16_t PWM)
{
    if(PWM == 0)
    {
        POCR0RA = 0x0FFF;
    } else {
        POCR0RA = 0;
    }
    POCR0SA = PWM;
}