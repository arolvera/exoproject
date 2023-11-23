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

#ifndef COMMON_COMMUNICATION_H
#define	COMMON_COMMUNICATION_H

#include "mcu_include.h"

typedef health_table_entry_t *health_array[HEALTH_ENTRIES];

void cc_sync(uint8_t *fifo, health_array health, uint8_t id) __attribute((weak));
void cc_initial_boot_report(uint8_t id);
void cc_error_report(uint8_t error, uint16_t adc_val, uint8_t id);
void cc_state_change_report(uint8_t state, uint8_t reason, uint8_t id) __attribute((weak));
void cc_version_report(uint8_t id) __attribute((weak));
void cc_scaling_report(float scaling, uint8_t description, uint8_t id) __attribute((weak));
void cc_command_processor(command_table_t *table, uint8_t *data, uint8_t id) __attribute((weak));

#endif	/* COMMON_COMMUNICATION_H */

