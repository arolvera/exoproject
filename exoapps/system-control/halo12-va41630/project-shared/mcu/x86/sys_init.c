// Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
//
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//  distribution, or disclosure of this software is expressly forbidden.
//
//  This Copyright notice may not be removed or modified without prior written
//  consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//  ExoTerra Corp
//  7640 S. Alkire Pl.
//  Littleton, CO 80127
//  USA
//
//  Voice:  +1 1 (720) 788-2010
//  http:   www.exoterracorp.com
//  email:  contact@exoterracorp.com
//

#define DECLARE_GLOBALS
#include "ext_decl_define.h"
#include "definitions.h"
#include "device.h"

/**
 * Sets up fpu, vector table and default clk. First function called on boot.
 */
void sys_init(void)
{
    //Need to use somthing in definition.h to make sure the compiler does not optimize it out
    gpio_init(&gpio_mcu_shutdown, GPIO_OUTPUT) ;
}
