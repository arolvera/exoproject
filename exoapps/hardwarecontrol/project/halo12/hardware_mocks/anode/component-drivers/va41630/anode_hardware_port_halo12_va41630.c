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

#include "anode_event_handlers.h"
#include "anode_hardware_port.h"
#include "anode_mcu.h"
#include "hardware_port/hal_hardware_port.h"

#define OVER_CURRENT       2  // PORTA[2]
#define SPARK_DETECT       3  // PORTA[3]
#define CONTROL_ENABLE     4  // PORTG[4] (not PG6 like the schematic says)
#define INPUT_POWER        4  // PORTA[4]

void ahp_input_power_enable(bool enable)
{
    // Intentionally left blank
}

void ahp_spark_detect_enable(bool enable)
{
    // Intentionally left blank
}

void ahp_over_current_enable(bool enable)
{
    // Intentionally left blank
}

static void spark_detected_callback(void)
{
    // Intentionally left blank
}

static void over_current_callback(void)
{
    // Intentionally left blank
}

void anode_hardware_port_init(void)
{
    hardware_port_cb_register(spark_detected_callback, SPARK_DETECT);
}
