/*
           Copyright (C) 2021 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file, via any medium is strictly prohibited
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/


/* 
 * File:   control_thrust.h
 * Author: jmeyers
 * 
 * @Company
 * Exoterra
 * 
 * @File Name
 * client_thrust.c
 * 
 * @Summary
 * This module does the work of setting/getting/throttling to thrust setpoints.
 * 
 * @Description
 * @todo describe this
 * 
 * Created on July 29, 2021, 1:30 PM
 * 
 */

#ifndef CLIENT_THRUST_H
#define	CLIENT_THRUST_H

#include "ext_decl_define.h"
#include  "definitions.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define THRUST_POINTS_MAX 32
#define THRUST_TABLE_SIZE THRUST_POINTS_MAX

typedef enum {
    START_METHOD_GLOW,
    START_METHOD_HARD,
    START_METHOD_COUNT,
} start_method_t;

int thrust_cathode_lf_setpoint_get(uint32_t setpoint, float *value);
int thrust_cathode_lf_setpoint_set(uint32_t setpoint, float value);

/**
 * @brief Get the anode flow setpoint value for the indexed setpoint.
 * @param setpoint Index for the setpoint table
 * @param value Point to variable to write the value to
 * @return
 */
int thrust_anode_flow_setpoint_get(uint32_t setpoint, float *value);
int thrust_anode_flow_setpoint_set(uint32_t setpoint, float value);
int thrust_anode_v_setpoint_get(uint32_t setpoint, float *value);
int thrust_anode_v_setpoint_set(uint32_t setpoint, float value);
int thrust_anode_i_setpoint_get(uint32_t setpoint, float *value);
int thrust_anode_i_setpoint_set(uint32_t setpoint, float value);
int thrust_magnet_i_setpoint_get(uint32_t setpoint, float *value);
int thrust_magnet_i_setpoint_set(uint32_t setpoint, float value);
int thrust_magnet_ratio_setpoint_get(uint32_t setpoint, float *value);
int thrust_magnet_ratio_setpoint_set(uint32_t setpoint, float value);
int thrust_millinewtons_get(uint32_t setpoint, float *value);
int thrust_millinewtons_set(uint32_t setpoint, float value);
int thrust_start_method_get(uint32_t setpoint, start_method_t *value);
int thrust_start_method_set(uint32_t setpoint, start_method_t value);
int thrust_timeout_get(uint32_t setpoint, uint32_t *value);
int thrust_timeout_set(uint32_t setpoint, uint32_t value);
int thrust_hf_start_setpoint_get(uint32_t setpoint, uint32_t *value);
int thrust_hf_start_setpoint_set(uint32_t setpoint, uint32_t value);
uint32_t thrust_table_max_valid(void);
int thrust_power_set(uint32_t setpoint, float value);
int thrust_power_get(uint32_t setpoint, float *value);

typedef struct thrust_data {
    float cathode_lf;
    float anode_flow;
    float anode_v;
    float anode_i;
    float magnet_i;
    float magnet_ratio;
    float thrust;
    float power;
    start_method_t start_method;
    uint32_t timeout;
    uint32_t hf_start_setpoint;
} thrust_data_t;

//Define in control_setpoint_<project>.c
EXT_DECL thrust_data_t throttle_table[THRUST_TABLE_SIZE];

int thrust_table_entry_get(uint32_t setpoint, thrust_data_t **data);

#ifdef	__cplusplus
}
#endif

#endif	/* CLIENT_THRUST_H */

