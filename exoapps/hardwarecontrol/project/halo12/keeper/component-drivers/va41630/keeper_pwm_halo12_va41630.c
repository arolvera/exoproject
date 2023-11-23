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
#include "keeper_pwm.h"
#include "timer/hal_timer.h"

// TIMER_ZERO is the flyback PWM
// TIMER_ONE is the starter PWM
#define FLYBACK_PWM_FREQ  250 // 200kHz PWM
#define STARTER_PWM_FREQ  100 // 500kHz PWM
#define MAX_FLYBACK_PWM   (FLYBACK_PWM_FREQ / 2) // flyback PWM must never exceed 50% duty-cycle

void keeper_pwm_init(void)
{
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << TIMER_ZERO); // enable timer0 clock
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= (1 << TIMER_ONE);  // enable timer1 clock

    VOR_IOCONFIG->PORTA[0] = (1 << IOCONFIG_PORTA_FUNSEL_Pos); // Set PORTA[0] to function one (timer0)
    VOR_IOCONFIG->PORTA[1] = (1 << IOCONFIG_PORTA_FUNSEL_Pos); // Set PORTA[1] to function one (timer1)
    VOR_GPIO->BANK[0].DIR |= (1 << 0); // set PORTA[0] to output
    VOR_GPIO->BANK[0].DIR |= (1 << 1); // set PORTA[1] to output

    VOR_TIM0->CTRL |= (0b110 << TIM0_CTRL_STATUS_SEL_Pos); // set timer0 to PWMA active
    VOR_TIM1->CTRL |= (0b110 << TIM1_CTRL_STATUS_SEL_Pos); // set timer1 to PWMA active
    VOR_TIM0->RST_VALUE = FLYBACK_PWM_FREQ;                // set timer0 to 200kHz
    VOR_TIM1->RST_VALUE = STARTER_PWM_FREQ;                // set timer1 to 500kHz
    VOR_TIM0->PWMA_VALUE = 0;                              // set timer0 low
    VOR_TIM1->PWMA_VALUE = 0;                              // set timer1 low
    VOR_TIM0->ENABLE |= (1 << TIM0_CTRL_ENABLE_Pos);       // enable timer0
    VOR_TIM1->ENABLE |= (1 << TIM1_CTRL_ENABLE_Pos);       // enable timer1
}

uint16_t kp_get_max_pwm(void)
{
    return MAX_FLYBACK_PWM;
}

void kp_set_flyback_pwm(uint16_t pwm)
{
    if(pwm > MAX_FLYBACK_PWM){
        pwm = MAX_FLYBACK_PWM;
    }
    VOR_TIM0->PWMA_VALUE = pwm;
}

void kp_starter_enable(bool enable)
{
    if(enable){
        VOR_TIM1->PWMA_VALUE = (STARTER_PWM_FREQ / 4); // set timer1 to a fixed 25% duty-cycle
    } else {
        VOR_TIM1->PWMA_VALUE = 0;
    }
}