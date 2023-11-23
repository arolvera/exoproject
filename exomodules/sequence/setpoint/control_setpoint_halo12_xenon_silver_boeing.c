/**
 * @file    control_setpoint_halo12_xenon_silver_boeing.c
 *
 * @brief   Implementation for controlling thruster Ready mode (Keeper lit) and Steady
 * state operation (thrusting at a setpoint).
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

#include "control_setpoint.h"
#define DECLARE_GLOBALS
#include "ext_decl_define.h"

/**
 * @brief The throttle table has settings for each throttle level defined for Halo12.
 * For reasons not entirely clear, most of the settings are not used. They appear to be
 * for reference only but it is not clear who would reference or when. The settings that
 * are actually used:
 * - cathode_lf - low flow valve psi to the keeper
 * - anode_flow - valve psi to the anode
 * - anode_v - voltage to the anode
 * - magnet_i - current to the magnet coil
 */
thrust_data_t throttle_table[THRUST_TABLE_SIZE] = {
/* 1 */ {.cathode_lf = 00.00, .anode_flow = 00.00, .anode_v = 200.0, .anode_i = 1.45, .magnet_i = 2.60, .magnet_ratio = 1.00, .thrust =  23.00, .power =  290.00, .start_method = 0, .timeout  = 0, .hf_start_setpoint = 99000},
/* 2 */ {.cathode_lf = 00.00, .anode_flow = 00.00, .anode_v = 200.0, .anode_i = 2.75, .magnet_i = 2.20, .magnet_ratio = 1.00, .thrust =  41.00, .power =  550.00, .start_method = 0, .timeout  = 0, .hf_start_setpoint = 99000},
/* 3 */ {.cathode_lf = 00.00, .anode_flow = 00.00, .anode_v = 400.0, .anode_i = 1.68, .magnet_i = 2.40, .magnet_ratio = 1.00, .thrust =  37.00, .power =  670.00, .start_method = 0, .timeout  = 0, .hf_start_setpoint = 99000},
/* 4 */ {.cathode_lf = 00.00, .anode_flow = 00.00, .anode_v = 400.0, .anode_i = 2.13, .magnet_i = 2.40, .magnet_ratio = 1.00, .thrust =  49.00, .power =  850.00, .start_method = 0, .timeout  = 0, .hf_start_setpoint = 99000},
/* 5 */ {.cathode_lf = 00.00, .anode_flow = 00.00, .anode_v = 400.0, .anode_i = 2.55, .magnet_i = 2.40, .magnet_ratio = 1.00, .thrust =  60.00, .power =  1020.00, .start_method = 0, .timeout  = 0, .hf_start_setpoint = 99000},
};
