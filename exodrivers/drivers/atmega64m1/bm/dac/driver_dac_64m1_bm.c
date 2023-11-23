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
#include "dac/hal_dac.h"

inline void dac_set(uint16_t DAC_value, uint8_t channel)
{
    uint16_t DAC_masked;
    DAC_masked = DAC_value & 0x03FF;   // Mask number to 10 bits max
    DACL = (uint8_t)(DAC_masked << 6); // Value is setup as left aligned
    DACH = (uint8_t)(DAC_masked >> 2); // to allow auto trigger
}

void dac_common_init(uint8_t channel)
{
    uint8_t dacon = 0;
    dacon |= (1 << DALA);  // left adjust
    dacon |= (1 << DAOE);  // output enable
    dacon |= (1 << DAEN);  // enable DAC
    DACON = dacon;
    dac_set(0, channel);
}
