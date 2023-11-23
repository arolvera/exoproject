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

#include "adc/hal_adc.h"
#include "device.h"
#include "timer/hal_timer.h"

#define ONE_HUNDRED_US         5000
#define FIFTY_US               2500

void adc_enable(uint8_t first_channel)
{
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << TIMER_TWO); // enable timer2 clock
    VOR_TIM2->RST_VALUE = FIFTY_US;                    // set the timer to interrupt every 50us
    timer_enable_interrupts(TIMER_TWO, true);// enable interrupts on timer2
    VOR_IRQ_ROUTER->ADCSEL = TIMER_TWO;                // set timer2 interrupts to trigger ADC conversion
    timer_start(TIMER_TWO, true);        // enable timer2

    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_ADC;          // enable the ADC clock
    VOR_CLKGEN->CTRL1 |= (0x00 << CLKGEN_CTRL1_ADC_CLK_DIV_SEL_Pos); // set the ADC clock divider
    VOR_ADC->IRQ_ENB |= (1 << ADC_IRQ_ENB_ADC_DONE_Pos);             // enable ADC interrupts
    VOR_ADC->FIFO_CLR |= (1 << ADC_FIFO_CLR_FIFO_CLR_Pos);           // clear the fifo
    VOR_ADC->CTRL |= (1 << first_channel);                           // start at the first channel
    NVIC_EnableIRQ(ADC_IRQn);                                   // enable the ADC NVIC entry
    NVIC_SetPriority(ADC_IRQn, 0);                       // set the interrupt priority
    VOR_ADC->CTRL |= (1 << ADC_CTRL_EXT_TRIG_EN_Pos);                // set the adc conversion to trigger on external interrupt (timer2)
}

void ADC_IRQHandler(void)
{
    VOR_ADC->IRQ_CLR |= (1 << ADC_IRQ_CLR_ADC_DONE_Pos); // clear the interrupt
    uint8_t new_channel = adc_interrupt_callback(VOR_ADC->FIFO_DATA);// call the generic callback

    VOR_ADC->CTRL &= ~(ADC_CTRL_CHAN_EN_Msk);             // clear the mux
    VOR_ADC->FIFO_CLR |= (1 << ADC_FIFO_CLR_FIFO_CLR_Pos);// clear the fifo
    VOR_ADC->CTRL |= (1 << new_channel);                  // set the ADC to the new channel
}
