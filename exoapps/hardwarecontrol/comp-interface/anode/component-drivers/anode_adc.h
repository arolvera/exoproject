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

#ifndef ANODE_ADC_H
#define	ANODE_ADC_H

#include <stdbool.h>

void anode_adc_init(void) __attribute__((weak));
void anode_adc_deinit(void) __attribute__((weak));
void aa_over_current_enable(bool enable);
void aa_get_data(void) __attribute__((weak));

#endif	/* ANODE_ADC_H */

