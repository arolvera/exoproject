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

#include "dac/hal_dac.h"
#include "device.h"

void dac_set(uint16_t DAC_value, uint8_t channel)
{
    uint16_t DAC_masked;
    DAC_masked = DAC_value & 0x0FFF;   // Mask number to 12 bits max
    switch(channel) {
        case 0:
            VOR_DAC0->FIFO_DATA = DAC_masked;
            VOR_DAC0->CTRL0 |= (1 << DAC0_CTRL0_MAN_TRIG_EN_Pos);
            break;
        case 1:
            VOR_DAC1->FIFO_DATA = DAC_masked;
            VOR_DAC1->CTRL0 |= (1 << DAC0_CTRL0_MAN_TRIG_EN_Pos);
            break;
    }
}

void dac_common_init(uint8_t channel)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_DAC;// enable the DAC clock
    switch(channel) {
        case 0:
            VOR_DAC0->FIFO_CLR |= (1 << DAC_FIFO_CLR_FIFO_CLR_Pos);// clear the fifo
            VOR_DAC0->CTRL1 |= (1 << DAC0_CTRL1_DAC_EN_Pos);       // enable the DAC
            break;
        case 1:
            VOR_DAC1->FIFO_CLR |= (1 << DAC_FIFO_CLR_FIFO_CLR_Pos);
            VOR_DAC1->CTRL1 |= (1 << DAC1_CTRL1_DAC_EN_Pos);
            break;
    }
    dac_set(0, channel);
}