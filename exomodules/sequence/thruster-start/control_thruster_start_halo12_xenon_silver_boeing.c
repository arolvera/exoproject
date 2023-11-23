/**
 * @file    control_thruster_start_halo12_xenon_silver_boeing.c
 *
 * @brief   Implementation of sequence engine tables for Halo 12.
 *
 * @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#include "control_thruster_start.h"
#define DECLARE_GLOBALS
#include "ext_decl_define.h"

/**
 * @brief All steps of the sequence specify error function 2 which powers down
 * all peripherals, keeping MCU's powered, and closes valves putting us back
 * into standby mode. All abort codes are fixed at 0x1E until we determine
 * that a different code is actually informative.
 */
sequence_array_t sequence_steady_state[SEQUENCE_MAX_STEPS_ANODE] = {
    0x0504050100000001,
    0x0001050100001388,
    0x020A050100000005,
    0x020C0501000005DC,
    0x0502050100000000,
    0x0104050100000000,
    0x0005050100000000,
    0, /* EOL       */
};

/**
 * @brief All steps of the sequence specify error function 2 which powers down
 * all peripherals, keeping MCU's powered, and closes valves putting us back
 * into standby mode. All abort codes are fixed at 0x1E until we determine
 * that a different code is actually informative.
 */
sequence_array_t sequence_ready_mode[SEQUENCE_MAX_STEPS_KEEPER] = {
    0x050104040000058C,
    0x0503040400004F7E,
    0x05020404000182B8,
    0x02010404000493E0,
    0x03090404000004B0,
    0x0303040400000000,
    0x010104040002AB98,
    0x01020404000002EE,
    0, /* EOL */
};
