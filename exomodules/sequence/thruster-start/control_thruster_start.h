/**
 * @file    control_thruster_start.h
 *
 * @brief   Interface for controlling thruster Ready mode (Keeper lit) and Steady
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

#ifndef CONTROL_THRUSTER_START_H
#define CONTROL_THRUSTER_START_H

#include "ext_decl_define.h"
#include "sequence/control_sequence.h"

#ifdef __cplusplus
extern "C" {
#endif

//Define in control_thruster_start_<project>.c
#define NAME_STEADY_STATE "Steady state"
EXT_DECL sequence_array_t sequence_steady_state[SEQUENCE_MAX_STEPS_ANODE];

//Define in control_thruster_start_<project>.c
#define NAME_READY_MODE "ready mode"
EXT_DECL sequence_array_t sequence_ready_mode[SEQUENCE_MAX_STEPS_KEEPER];

/**
 * @brief Move to Ready mode by lighting the Keeper and getting it to a stable
 * operating point.
 * @return Line number in code where an error took place.
 */
int ctrl_ts_ready_mode_run(void);

/**
 * @brief Get the status of the transition to Ready mode.
 * @return A status as defined in sequence_status_t
 */
uint32_t ctrl_ts_ready_mode_stat(void);

/**
 * @brief Move from Ready mode to Steady State by throttling to a specified setpoint.
 * @param setpoint Index into a setpoint table with values to throttle to.
 * @return Line number in code where an error took place.
 */
int ctrl_ts_steady_state_run(uint32_t setpoint);

/**
 * @brief Get the status of the transition to Steady State.
 * @return A status as defined in sequence_status_t
 */
uint32_t ctrl_ts_steady_state_stat(void);

/**
 * @brief This function is not used anywhere and is not implemented. Was the purpose to
 * allow creating or modifying a sequence from the messaging protocol?
 *
 * ToDo: complete the implementation or get rid of this.
 *
 * @param seq
 * @param step
 * @param entry_upper_32
 * @param entry_lower_32
 *
 * @return Line number in code where an error took place.
 */
int ctrl_ts_entry_write(sequence_t seq, uint8_t step, uint32_t entry_upper_32, uint32_t entry_lower_32);

/**
 * @brief Return the Ready Mode sequence value at the given index.
 *
 * ToDo: This is not used anywhere. Complete the implementation or get rid of it.
 *
 * @param index index requested
 * @return 0 on success, -1 otherwise
 */
uint32_t ctrl_ts_ready_mode_get(unsigned int index);

/**
 * @brief Edit a value in the Steady State sequence
 *
 * ToDo: This is not used anywhere. Complete the implementation or get rid of it.
 *
 * @param index array index
 * @param value new value
 * @return 0 on success, -1 if the index is out of bounds
 */
int ctrl_ts_ready_mode_set(unsigned int index, uint64_t value);

/**
 * @brief Return the Steady State sequence value at the given index.
 *
 * ToDo: This is not used anywhere. Complete the implementation or get rid of it.
 *
 * @param index index requested
 * @return 0 on success, -1 otherwise
 */
uint32_t ctrl_ts_steady_state_get(unsigned int index);

/**
 * @brief Edit a value in the Steady State sequence.
 *
 * ToDo: This is not used anywhere. Complete the implementation or get rid of it.
 *
 * @param index array index
 * @param value new value
 * @return 0 on success, -1 if the index is out of bounds
 */
int ctrl_ts_steady_state_set(unsigned int index, uint64_t value);

/**
 * @brief Get the status of a transition to Ready mode or Steady State. Just looking to see
 * if the transition is still in progress.
 * @return 0 (TCS_CO_INVALID) if not in transition, TCS_TRANSITION_READY_MODE or
 * TCS_TRANSITION_STEADY_STATE if it is.
 */
uint32_t ctrl_ts_in_transition(void);

/**
 * @brief Inspect or update the values of a sequence step.
 *
 * @param table The sequence table being update
 * @param step The step number in the sequence table
 * @param val Value to write or the result of a read
 * @param which Update upper 32 bits or lower 32 bits of the sequence step
 * @param rw Read or write the value
 * @return Line number in code where an error took place.
 */
int ctrl_ts_step_rw(unsigned int table, unsigned int step, uint32_t *val, unsigned int which, unsigned int rw);

#ifdef __cplusplus
}
#endif

#endif /* CONTROL_THRUSTER_START_H */
