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

#include "hardware_port/hal_hardware_port.h"
#include "keeper_event_handlers.h"
#include "keeper_hardware_port.h"
#include "keeper_mcu.h"

#define SPARK_DETECT       3 // PORTA[3]

void khp_spark_detect_enable(bool enable)
{
}

static void spark_detected_callback(void)
{
    keeper.common.current_state = keeper_spark_detected_handler();
}

void keeper_hardware_port_init(void)
{
    hardware_port_cb_register(spark_detected_callback, SPARK_DETECT);
}
