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

#include "valve_pwm.h"
#include <avr/io.h>
#include <stddef.h>

void valve_PSC_init(void)
{
    uint8_t pllcsr = 0;
    pllcsr |= (1 << PLLF); // 64MHz clock
    pllcsr |= (1 << PLLE); // enable
    PLLCSR = pllcsr;
    
    uint8_t poc = 0;
    poc |= (1 << POEN0A); //enable 0A
    poc |= (1 << POEN1A); //enable 1A
    poc |= (1 << POEN2A); //enable 2A
    POC = poc;
    
    uint8_t pcnf = 0;
    pcnf |= (1 << POPA);  // A active high
    pcnf &= (1 << POPB);
    PCNF = pcnf;
    
    PCTL |= (1 << PCLKSEL); // fast clock
    
    POCR0RA = 0x0FFF;
    POCR0SA = 0;
    POCR1RA = 0x0FFF;
    POCR1SA = 0;
    POCR2RA = 0x0FFF;
    POCR2SA = 0;
    POCR_RB = VALVE_MAX_PWM; // 40kHz PWM
    
    PMIC0 |= (1 << POVEN0); // disable overlap protection
    PMIC1 |= (1 << POVEN1); // disable overlap protection
    PMIC2 |= (1 << POVEN2); // disable overlap protection
    
    PCTL |= (1 << PRUN);    //run PSC
}

void vp_set_PWM(uint16_t PWM, valve_types_t valve)
{
    switch (valve) 
    {
        case ANODE_FLOW_VALVE:
            if(PWM == 0)
            {
                POCR0RA = 0x0FFF;
            } else {
                POCR0RA = 0;
            }
            POCR0SA = PWM;
            break;
        case CATHODE_HIGH_FLOW_VALVE:
            if(PWM == 0)
            {
                POCR1RA = 0x0FFF;
            } else {
                POCR1RA = 0;
            }
            POCR1SA = PWM;
            break;
        case CATHODE_LOW_FLOW_VALVE:
            if(PWM == 0)
            {
                POCR2RA = 0x0FFF;
            } else {
                POCR2RA = 0;
            }
            POCR2SA = PWM;
            break;
    }
}
