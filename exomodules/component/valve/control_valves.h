/**
 * @file    control_valves.h
 *
 * @brief   Interface for valve commands common to both Halo 6 and 12.
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

#ifndef CONTROL_VALVES_H
#define CONTROL_VALVES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <math.h>                   // For rounding floats

#include "definitions.h"            // Harmony definitions
#include "client_p.h"       // Private include for client control
#include "valve_mcu.h"              // Shared header with Valve MCU


    
/* Default values */
#define DEFUALT_VALVES_CATHODE_HF           0
#define DEFUALT_VALVES_CATHODE_LF           0
#define DEFUALT_VALVES_ANODE_FLOW           0
#define DEFUALT_VALVES_LATCH_VALVE          OFF_SET_POINT
#define DEFAULT_VALVES_STATE                VS_POWERED_OFF
#define DEFAULT_VALVES_STATE_STAT           V_MSG_NO_ERROR

/* Pressures are in human readable values.
 * Factored values are what is calculated for the Atmega,
 * so they do not have to do work */
typedef struct control_valves_t_ {
    float       cathode_hf;
    uint16_t    cathode_hf_factored;
    float       cathode_lf;
    uint16_t    cathode_lf_factored;
    float       anode_flow;
    uint16_t    anode_flow_factored;
    uint8_t     latch_valve;
    control_state_t stat;
} control_valves_t;

typedef enum {
    CONTROL_VALVES_CATHODE_HF           = (1 << 0),
    CONTROL_VALVES_CATHODE_HF_FACTORED  = (1 << 1),
    CONTROL_VALVES_CATHODE_LF           = (1 << 2),
    CONTROL_VALVES_CATHODE_LF_FACTORED  = (1 << 3),
    CONTROL_VALVES_ANODE_FLOW           = (1 << 4),
    CONTROL_VALVES_ANODE_FLOW_FACTORED  = (1 << 5),
    CONTROL_VALVES_LATCH_VALVE          = (1 << 6),
    CONTROL_VALVES_SET_ALL              =((1 << 7) - 1),
} CONTROL_VAVLES_CMD_MASK;

int ctrl_valves_init();
int ctrl_valves_reinit(void);
int ctrl_valves_version_get(uint32_t** version);

uint16_t ctrl_valve_psi_to_counts_forty(float psi);
float ctrl_valve_counts_to_psi_forty(uint16_t counts);
uint16_t ctrl_valve_psi_to_counts_three_thousand(float psi);
float ctrl_valve_counts_to_psi_three_thousand(uint16_t counts);
uint16_t ctrl_valve_duty_cycle_to_PSC_setpoint(uint16_t duty_cycle);
float ctrl_valve_counts_to_volts(uint16_t counts);
float ctrl_valve_counts_to_temperature(uint16_t counts);

uint16_t ctrl_valves_cath_pressure(void);
void ctrl_valves_error_handler_init(void);

int      ctrl_valves_cathode_hf_set(float psi);
float    ctrl_valves_cathode_hf_get();
uint16_t ctrl_valves_cathode_hf_factored_get();

int      ctrl_valves_cathode_lf_set(float psi);
float    ctrl_valves_cathode_lf_get();
uint16_t ctrl_valves_cathode_lf_factored_get();

int      ctrl_valves_anode_flow_set(float psi);
float    ctrl_valves_anode_flow_get();
uint16_t ctrl_valves_anode_flow_factored_get();

int     ctrl_valves_latch_valve_set(uint8_t open);
uint8_t ctrl_valves_latch_valve_get();
int     ctrl_valves_set(control_valves_t *c);

int     ctrl_valves_get(control_valves_t *c);



#ifdef __cplusplus
}
#endif

#endif /* CONTROL_VALVES_H */
