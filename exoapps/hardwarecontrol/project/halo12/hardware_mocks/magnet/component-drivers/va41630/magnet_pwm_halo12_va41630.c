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

#include "magnet_pwm.h"
#include "timer/hal_timer.h"

// TIMER_FIFTEEN is the command PWM
#define PWM_FREQ          500 // 100kHz PWM

// note: timer16 is clocked from the system clock divided by four, but timer15 is divided by two,
// which is why the timer16 stuff has an extra divide by two
void magnet_pwm_init(void)
{

}

uint16_t mp_get_max_pwm(void)
{
    return PWM_FREQ;
}

void mp_set_pwm(uint16_t pwm)
{

}