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
#include "valve_pwm.h"
#include "timer/hal_timer.h"

// TIMER_ZERO is the cat hf PWM
#define CAT_HF_PWM_FREQ   1000 // 50kHz PWM

void valve_pwm_init(void)
{
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << TIMER_ZERO); // enable timer0 clock

    VOR_IOCONFIG->PORTA[0] = (1 << IOCONFIG_PORTA_FUNSEL_Pos); // Set PORTA[0] to function one (timer0)
    VOR_GPIO->BANK[0].DIR |= (1 << 0); // set PORTA[0] to output

    VOR_TIM0->CTRL |= (0b110 << TIM0_CTRL_STATUS_SEL_Pos); // set timer0 to PWMA active
    VOR_TIM0->RST_VALUE = CAT_HF_PWM_FREQ;                 // set timer0 to 50kHz
    VOR_TIM0->PWMA_VALUE = 0;                              // set timer0 low
    VOR_TIM0->ENABLE |= (1 << TIM0_CTRL_ENABLE_Pos);       // enable timer0
}

void vp_set_pwm(uint16_t pwm)
{
    if(pwm > CAT_HF_PWM_FREQ){
        pwm = CAT_HF_PWM_FREQ;
    }
    VOR_TIM0->PWMA_VALUE = pwm;
}
