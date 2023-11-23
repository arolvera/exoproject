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

#include <stdint.h>
#include "valve_hardware_port.h"
#include "valve_timer.h"

#define LV_OPEN        3 // PORTA[3]
#define LV_CLOSE       1 // PORTA[1]

void vhp_lv_clear(void)
{

}

void vhp_lv_open(void)
{
    vt_start();
}

void vhp_lv_close(void)
{

    vt_start();
}

void valve_hardware_port_init(void)
{
    vhp_lv_clear();       // Set LV outputs low
}
