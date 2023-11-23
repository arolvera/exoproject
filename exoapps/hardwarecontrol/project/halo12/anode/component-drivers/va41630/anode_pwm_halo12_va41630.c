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

#include "device.h"
#include "anode_pwm.h"
#include "timer/hal_timer.h"

// TIMER_ZERO is the X PWM
// TIMER_ONE is the Y PWM
#define TIM_CAS_SRC_TIM_3         83  // start trigger for the X and Y PWMs (timer3)
#define QUADRATIC_BOOST_PWM_FREQ  385 // 130kHz PWM

void anode_pwm_init(void)
{
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << TIMER_ZERO); // enable timer0 clock
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << TIMER_ONE);  // enable timer1 clock

    VOR_IOCONFIG->PORTA[0] = (1 << IOCONFIG_PORTA_FUNSEL_Pos); // Set PORTA[0] to function one (timer0)
    VOR_IOCONFIG->PORTA[1] = (1 << IOCONFIG_PORTA_FUNSEL_Pos); // Set PORTA[1] to function one (timer1)
    VOR_GPIO->BANK[0].DIR |= (1 << 0); // set PORTA[0] to output
    VOR_GPIO->BANK[0].DIR |= (1 << 1); // set PORTA[1] to output

    VOR_TIM0->CTRL |= (0b100 << TIM0_CTRL_STATUS_SEL_Pos); // set timer0 to PWMB
    VOR_TIM1->CTRL |= (0b100 << TIM1_CTRL_STATUS_SEL_Pos); // set timer1 to PWMB

    // The goal here is to get two perfectly interleaved PWMs, so they need to be cascaded.
    // As usual, the registers don't seem to do exactly what the datasheet says, but this gets
    // the desired result. I am going to leave it as is, but there may be a better way.
    VOR_TIM0->CSD_CTRL |= (1 << TIM0_CSD_CTRL_CSDEN0_Pos);
    VOR_TIM0->CSD_CTRL |= (1 << TIM0_CSD_CTRL_CSDTRG0_Pos);
    VOR_TIM0->CASCADE0 = TIM_CAS_SRC_TIM_3;
    VOR_TIM1->CSD_CTRL |= (1 << TIM1_CSD_CTRL_CSDEN0_Pos);
    VOR_TIM1->CSD_CTRL |= (1 << TIM1_CSD_CTRL_CSDTRG0_Pos);
    VOR_TIM1->CASCADE0 = TIM_CAS_SRC_TIM_3;

    VOR_TIM0->RST_VALUE = QUADRATIC_BOOST_PWM_FREQ;        // set timer0 to 130kHz
    VOR_TIM1->RST_VALUE = QUADRATIC_BOOST_PWM_FREQ;        // set timer1 to 130kHz

    VOR_TIM0->CNT_VALUE = QUADRATIC_BOOST_PWM_FREQ;
    VOR_TIM1->CNT_VALUE = (QUADRATIC_BOOST_PWM_FREQ / 2);  // interleave the PWMs

    VOR_TIM0->PWMA_VALUE = 0;                              // set timer0 low
    VOR_TIM1->PWMA_VALUE = 0;                              // set timer1 low
    VOR_TIM0->ENABLE |= (1 << TIM0_CTRL_ENABLE_Pos);       // enable timer0
    VOR_TIM1->ENABLE |= (1 << TIM1_CTRL_ENABLE_Pos);       // enable timer1
}

void ap_enable_pwm(void)
{
    VOR_TIM0->PWMA_VALUE = (QUADRATIC_BOOST_PWM_FREQ / 2) + 1; // fixed 50% duty-cycle
    VOR_TIM1->PWMA_VALUE = (QUADRATIC_BOOST_PWM_FREQ / 2);
}

uint16_t ap_get_max_pwm(void)
{
    return QUADRATIC_BOOST_PWM_FREQ;
}

void ap_set_pwm(uint16_t pwm, uint8_t channel)
{
    if(pwm < QUADRATIC_BOOST_PWM_FREQ) {
        if(pwm > (QUADRATIC_BOOST_PWM_FREQ * 0.9)){
            pwm = (QUADRATIC_BOOST_PWM_FREQ * 0.9); // limit to 90% duty-cycle
        }
        if(channel == X_PWM){
            if(pwm == 0) {
                VOR_TIM0->PWMA_VALUE = 0;// set timer0 low
            } else {
                VOR_TIM0->PWMA_VALUE = QUADRATIC_BOOST_PWM_FREQ + (pwm);
                VOR_TIM0->PWMB_VALUE = QUADRATIC_BOOST_PWM_FREQ - (pwm);
            }
        }
        if(channel == Y_PWM){
            if(pwm == 0) {
                VOR_TIM1->PWMA_VALUE = 0;// set timer1 low
            } else {
                VOR_TIM1->PWMA_VALUE = QUADRATIC_BOOST_PWM_FREQ + (pwm);
                VOR_TIM1->PWMB_VALUE = QUADRATIC_BOOST_PWM_FREQ - (pwm);
            }
        }
    }
}

void ap_disable_pwm(void)
{
    VOR_TIM0->PWMA_VALUE = QUADRATIC_BOOST_PWM_FREQ + 1;
    VOR_TIM1->PWMA_VALUE = 0;
}