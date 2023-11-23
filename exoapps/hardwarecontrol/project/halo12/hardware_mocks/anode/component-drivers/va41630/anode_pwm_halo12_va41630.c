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


#include "anode_pwm.h"
#include "timer/hal_timer.h"

// TIMER_ZERO is the X PWM
// TIMER_ONE is the Y PWM
#define TIM_CAS_SRC_TIM_3         83  // start trigger for the X and Y PWMs (timer3)
#define QUADRATIC_BOOST_PWM_FREQ  385 // 130kHz PWM

void anode_pwm_init(void)
{
    // Intentionally left blank
}

void ap_enable_pwm(void)
{
    // Intentionally left blank
}

uint16_t ap_get_max_pwm(void)
{
    return QUADRATIC_BOOST_PWM_FREQ;
}

void ap_set_pwm(uint16_t pwm, uint8_t channel)
{
    // Intentionally left blank
}

void ap_disable_pwm(void)
{
    // Intentionally left blank
}