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

#ifndef MAGNET_PWM_H
#define	MAGNET_PWM_H
#include <stdint.h>

void magnet_pwm_init(void) __attribute__((weak));
uint16_t mp_get_max_pwm(void);
void mp_set_pwm(uint16_t pwm);
void mp_set_inner_PWM(uint16_t PWM) __attribute__((weak));
void mp_set_outer_PWM(uint16_t PWM) __attribute__((weak));

#endif	/* MAGNET_PWM_H */

