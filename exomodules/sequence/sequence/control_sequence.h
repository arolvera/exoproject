/**
 * @file    control_sequence.h
 *
 * @brief   Interface for the Halo and Halo 12 sequence engine. The sequence
 * engine is a feature that allows execution of a configurable sequence of
 * commands and events through a table-diven design approach.
 *
 * Rules of the road
 * - Sequence functions should all be generic and know nothing about internal
 *   workings of the control modules.  For example, this ONLY uses functions
 *   like keeper_is_running(), and not keeper_state == KS_CURRENT_MODE.  The
 *   details of "is running" should be abstracted in the control module.
 *
 * - All sequence functions return 0 on success and non-zero otherwise.  There
 *   cannot be any further meaning to sequence function return values.  The
 *   function succeeded or failed.  The preferred return code for the failure
 *   is the code the __LINE__ on which it failed or the non-zero return value
 *   of a function called from here.
 *
 * - One failure aborts everything.  It will abort the current sequence and
 *   any sequence that may be queued next.  It is up to the higher level logic
 *   to figure it out.
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

#ifndef CONTROL_SEQUENCE_H
#define CONTROL_SEQUENCE_H

#include "definitions.h"                // Harmony definitions
#include "utils/macro_tools.h" // for SIZEOF

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#define SEQUENCE_MAX_STEPS          32
#define SEQUENCE_MAX_STEPS_ANODE    SEQUENCE_MAX_STEPS
#define SEQUENCE_MAX_STEPS_KEEPER   SEQUENCE_MAX_STEPS
#define SEQUENCE_MAX_STEPS_BIT      SEQUENCE_MAX_STEPS
#define SEQUENCE_MAX_STEPS_THROTTLE  16
#define UPPER_32               0
#define LOWER_32               1
#define SEQ_READ               0
#define SEQ_WRITE              1

typedef enum {
    SEQ_MODE_READY,
    SEQ_MODE_STEADY_STATE,
    /* New modes must go above SEQ_MODE_EOL */
    SEQ_MODE_EOL,
    SEQ_THROTTLE_1,
    SEQ_THROTTLE_2,
    SEQ_THROTTLE_3,
    SEQ_THROTTLE_4,
    SEQ_THROTTLE_5,
    SEQ_THROTTLE_6,
    /* New throttle tables must go above SEQ_EOL */
    SEQ_EOL,
    SEQ_COND_MAGS,
    SEQ_COND_KEEPER_1,
    SEQ_COND_KEEPER_2,
    SEQ_COND_KEEPER_3,
    SEQ_COND_KEEPER_4,
    SEQ_COND_ANODE_1,
    /* New conditioning tables must go above SEQ_COND_EOL */   
    SEQ_COND_EOL,
    SEQ_BIT_USER_MOD,
    SEQ_BIT_LATCH_VALVE_OPEN,
    SEQ_BIT_LATCH_VALVE_CLOSE,
    SEQ_BIT_CATHODE_LOW_FLOW_CHECK,
    SEQ_BIT_ANODE_VALVE_CHECK,
    SEQ_BIT_PCV_DRAIN,
    SEQ_BIT_INNER_COIL_TEST,
    SEQ_BIT_OUTER_COIL_TEST,
    SEQ_BIT_KEEPER_TEST,
    SEQ_BIT_ANODE_TEST,
    SEQ_CATH_LF_CHECK_AMBIENT,
    SEQ_ANODE_VALVE_CHECK_AMBIENT,
    SEQ_OPEN_ALL_VALVES,
    /* New conditioning tables must go above SEQ_COND_BIT_EOL */
    SEQ_BIT_EOL,
} sequence_t;
    
typedef enum {
    ABORT_UNKNOWN_ERROR,
    ABORT_KEEPER_ERROR,
    ABORT_ANODE_ERROR,
    ABORT_MAGNET_ERROR,
    ABORT_VALVE_ERROR,
    ABORT_MAX_ERROR = 0xff,
} sequence_abort_error_t;

typedef uint64_t sequence_array_t;
typedef struct {
    sequence_array_t *seq;
    uint32_t size;
    uint32_t *status;
    char *name;
    SemaphoreHandle_t *psem;
} sequence_run_t;

/* Sequence Error Definition */
typedef enum {
    SEQ_STAT_IDLE       = 0,
    SEQ_STAT_QUEUED     = 1,
    SEQ_STAT_RUNNING    = 2,
    SEQ_STAT_ERROR      = 3,
    SEQ_STAT_ABORTED    = 4,
    SEQ_STAT_SUCCESS    = 5,
} sequence_status_t;

typedef union {
        uint32_t sq_err;
}sq_control_specific_detail_t;

/* The status in the lower nibble */
#define SEQUENCE_STATUS_GET( __STAT__) ((uint32_t)( __STAT__ & 0x0F))
#define SEQUENCE_IS_RUNNING( __SEQ__ ) ((SEQUENCE_STATUS_GET(__SEQ__) == SEQ_STAT_RUNNING) || \
                                        (SEQUENCE_STATUS_GET(__SEQ__) == SEQ_STAT_QUEUED))



/**
 * @brief Ignition and throttle sequences use a setpoint lookup table to store values
 * for current and voltage. This function reads the current index into that table.
 * @return Index into the setpoint table
 */
uint32_t ctrl_sequence_setpoint_get();

/**
 * @brief Ignition and throttle sequences use a setpoint lookup table to store values
 * for current and voltage. This function sets a new index into that table.
 * @param sp New index
 */
void ctrl_sequence_setpoint_set(uint32_t sp);

/**
 * @brief Get the status of the currently running sequence.
 * @return One of the values in sequence_status_t
 */
uint32_t ctrl_sequence_status_get(void);

/**
 * @brief Force a sequence abort
 */
void ctrl_sequence_abort(void);

/**
 * @brief Set a sequence abort error. The error may or may not abort the current
 * sequence. If the current step has the corresponding bit set, the sequence will
 * abort, else it will not.
 * @param error Sequence error abort code (potential abort condition)
 * @return 0 = success, non-zero otherwise
 */
int ctrl_sequence_abort_error(sequence_abort_error_t error);

/**
 * @brief Turn off the Magnets, Anode, and Keeper. Then set all MCU's to defaults.
 * @return 0 = success, non-zero otherwise
 */
int ctrl_sequence_error_shutdown(void);

/**
 * @brief Shut down the thruster and return to Standby from Steady State.
 * This function is only called for Halo 12.
 * @return 0 = success, non-zero otherwise
 */
int ctrl_sequence_standby_return(void);

/**
 * @brief Start a new sequence.
 * @param pSeq New setpoint index
 * @return 0 = success, non-zero otherwise
 */
int ctrl_sequence_run(sequence_run_t *pSeq);

/**
 * @brief Start a throttle sequence. Sets a new setpoint index and then calls run.
 * @param pSeq Sequence structure to run
 * @param setpoint New setpoint index
 * @return 0 = success, non-zero otherwise
 */
int ctrl_sequence_throttle_run(sequence_run_t *pSeq, uint32_t setpoint);

/**
 * @brief Init and register error codes for error handling.
 */
void sq_control_error_handler_init(void);

/**
 * @brief Initialize the sequence engine and start the task.
 */
void ctrl_sequence_init(void);


/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* CONTROL_SEQUENCE_H */

/* *****************************************************************************
 End of File
 */
