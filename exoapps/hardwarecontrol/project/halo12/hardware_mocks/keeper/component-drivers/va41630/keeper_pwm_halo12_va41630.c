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

#include "keeper_pwm.h"
#include "timer/hal_timer.h"

// TIMER_ZERO is the flyback PWM
// TIMER_ONE is the starter PWM
#define FLYBACK_PWM_FREQ  250 // 200kHz PWM
#define STARTER_PWM_FREQ  100 // 500kHz PWM
#define MAX_FLYBACK_PWM   (FLYBACK_PWM_FREQ / 2) // flyback PWM must never exceed 50% duty-cycle

void keeper_pwm_init(void)
{
}

uint16_t kp_get_max_pwm(void)
{
    return MAX_FLYBACK_PWM;
}

void kp_set_flyback_pwm(uint16_t pwm)
{

}

void kp_starter_enable(bool enable)
{

}