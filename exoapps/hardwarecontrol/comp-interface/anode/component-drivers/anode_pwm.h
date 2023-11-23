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

#ifndef ANODE_PWM_H
#define	ANODE_PWM_H

#include <stdint.h>

#define X_PWM    0
#define Y_PWM    1

void anode_pwm_init(void) __attribute__((weak));
void ap_enable_pwm(void);
void ap_disable_pwm(void);
uint16_t ap_get_max_pwm(void);
void ap_set_pwm(uint16_t pwm, uint8_t channel) __attribute__((weak));

#endif	/* ANODE_PWM_H */

