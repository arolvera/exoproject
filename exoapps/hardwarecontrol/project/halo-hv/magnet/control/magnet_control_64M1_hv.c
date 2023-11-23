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

#include "component_communication.h"
#include "component_service.h"
#include "magnet_adc.h"
#include "magnet_command_handlers.h"
#include "magnet_control.h"
#include "magnet_state_handlers.h"
#include "magnet_timer.h"
#include <stddef.h>

static health_table_entry_t health_response_0[] = 
    DECLARE_MAGNET_HEALTH_ENTRY_0(&magnet.inner_current, 
                                  &magnet.inner_voltage, 
                                  &magnet.outer_current,    
                                  &magnet.outer_voltage);

static health_table_entry_t health_response_1[] = 
    DECLARE_MAGNET_HEALTH_ENTRY_1(&magnet.inner_PWM,
                                  &magnet.outer_PWM,
                                  &magnet.temperature,
                                  &common.current_state); 

static health_array magnet_health = {
    health_response_0,
    health_response_1,
    0
};

void mc_sync(uint32_t id, uint8_t dlc, volatile uint8_t *fifo)
{
    cc_sync(CAN_ID_MAGNET, dlc, fifo, magnet_health);
}

void magnet_control_init(void)
{
    common.error_code = MAGNET_NO_ERROR;
    common.error_ADC = 0;
    magnet.inner_PWM = 0;
    magnet.outer_PWM = 0;
}
