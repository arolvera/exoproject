/**
 * @file    control_sequence.c
 *
 * @brief   Implementation for the sequence engine.
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

#include <stdint.h>                     // standard ints

#include "client-control/client_control.h"// Access to client functions
#include "cmd/command_anode.h"
#include "cmd/command_keeper.h"
#include "cmd/command_magnets.h"
#include "control_sequence.h"// This module
#include "error/error_handler.h"
#include "keeper/control_keeper.h"
#include "magnet/control_magnets.h"
#include "anode/control_anode.h"
#include "magnet_mcu.h"
#include "setpoint/control_setpoint.h"
#include "sys/sys_timers.h"           // Timers for times events
#include "throttle/control_throttle.h"// Throttle to a setpiont
#include "trace/trace.h"              // Trace messages
#include "valve/control_valves.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif
  

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_SEQUENCE_SUBMODULE

#define QUEUE_ITEM_SIZE_SEQ  (sizeof(sequence_run_t))
#define BUFFER_COUNT_SEQ    10
static  QueueHandle_t       xSeqQueue;
static  StaticQueue_t       xSeqStaticQueue;
static uint8_t ucSeqQueueStorageArea[QUEUE_ITEM_SIZE_SEQ * BUFFER_COUNT_SEQ];

#define SEQ_PARM_FACTOR_FLOAT       1000.0

/* Default timeout for sequence functions, if the MCU does not respond within
 * this timeout, something is very wrong, it is not responding anymore
 */
#define SEQ_DEFAULT_TO 5000

/*
 * Default threshold for anode current where the keeper must be on for stability.
 */
#define ANODE_LOW_CURRENT_THRESHOLD         1.2

/* The full sequence step is stored in a 64 bit number, the upper 32 is table,
 * function, error, and abort info.   The lower 32 is the parameter info
 */
#define SEQUENCE_TO_FUNCTION_PARAMETERS(__X__) (__X__ >> 32)

/* Sequences are encoded in 32 bits
 * 31:28 - Table Number (Util, Anode, Keeper, etc)
 * 27:24 - Function number in the individual table
 * 23:00 - 24 bit integer parameter
 */
#define TABLE_NUM_POS   24
#define TABLE_NUM_MASK  (0xFF << TABLE_NUM_POS)
#define SEQUENCE_VALUE_TO_TABLE_NUM(__X__)  ((__X__ & TABLE_NUM_MASK) >> TABLE_NUM_POS)

#define FUNC_NUM_POS    16
#define FUNC_NUM_MASK   (0xFF << FUNC_NUM_POS)
#define SEQUENCE_VALUE_TO_FUNC_NUM(__X__)   ((__X__ & FUNC_NUM_MASK) >> FUNC_NUM_POS)

#define ERROR_NUM_POS    8
#define ERROR_NUM_MASK  (0xFF << ERROR_NUM_POS)
#define SEQUENCE_VALUE_TO_ERR_NUM(__X__)    ((__X__ & ERROR_NUM_MASK) >> ERROR_NUM_POS)

/* Abort events are an array of bit masks that indicate any sequence abort
 * events for a given step in the sequence.   The maximum number of abort event
 * bitmasks was determined by the number of bits left in the upper 32 bits of
 * the sequence step.  The function, table, and error occupy 6 bytes, leaving
 * 2 bytes for abort bitmasks.  Therefore, we have 255 different combanation
 * of 32 bit abort bitmaks. */
#define ABORT_MAX_EVENTS (1 << 8) /* Must be a power of 2 */
#define ABORT_MASK (ABORT_MAX_EVENTS - 1)

static uint32_t abort_sequence = 0;

/* Sequence status will carry the current setup in the upper bits, and the
 * current status in the lowest byte */
#define SEQUENCE_VALUE_TO_STATUS(__P_STAT__, __X__) \
    (*__P_STAT__) = ((__X__ & ~0xFFFF) | ((*__P_STAT__) & 0xFFFF))

/* Place status in the lower nibble */
#define SEQUENCE_STATUS_SET(__P_STAT__, __STAT__) \
    (*__P_STAT__) = ((*(__P_STAT__) & ~0x0F) | ( __STAT__ & 0x0F))

/* Place the last error in bits 3:1 */
#define SEQUENCE_ERROR_SET(__P_STAT__, __ERR__) \
    (*__P_STAT__) = ((*(__P_STAT__) & ~0xFFF0) | ((__ERR__ & 0xFFF) << 4))

/* Params live in the lower 32 of a 64 bit number */
#define PARM_MASK  ((1LL << 32) - 1)
#define SEQUENCE_VALUE_TO_PARM(__X__)       ((__X__ & PARM_MASK))

#define seq_TASK_PRIORITY       ( tskIDLE_PRIORITY + 2)
#define seq_TASK_STACK_SIZE     ( configMINIMAL_STACK_SIZE * 3 )

static StackType_t seq_task_stack[seq_TASK_STACK_SIZE];
static StaticTask_t seq_TaskBuffer;

static uint32_t abort_event_mask = 0;

/* seq_throttle_setpoint stores the setpoint we are running at and is updated
 * to a new setpoint index just prior to throttling to that index. However,
 * there are situations where throttling to a new setpoint can fail (i.e. keeper
 * needs to be lit and fails). Rather than shutting down, we will want to remain
 * at our current setpoint and will need to reset seq_throttle_setpoint to its
 * previous value. seq_throttle_setpoint_prev is created to store that previous
 * value. We do it here because the control_setpoint module doesn't track the
 * index value. */
static uint32_t seq_throttle_setpoint = 0;
static uint32_t seq_throttle_setpoint_prev = 0;

static sequence_run_t seq;

static int function_count(void *table);

/* Utility sequence functions - declarations */
static int sq_util_delay(void *parm);
static int sq_util_set_setpoint(void *parm);
static int sq_util_throttle(void *parm);
static int sq_util_throttle_setpoint(void *parm);

/* Error function - declarations */
static int sq_error_emcy_shutdown(void *parm);
static int sq_error_shutdown(void *parm);
static int sq_error_anode_shutdown(void *parm);
static int sq_error_mode_steady_state_shtdn(void* parm);
static int sq_error_mode_steady_state_rm(void* parm);
static int sq_error_mode_ready_mode_shtdn(void* parm);
static int sq_error_mode_throttling(void* parm);
static int sq_error_bit_shtdn(void* parm);
static int sq_error_conditioning_shtdn(void* parm);
static int sq_error_conditioning_rm(void* parm);

/* Keeper sequence functions - declarations */
static int sq_keeper_voltage_set(void *parm);
static int sq_keeper_current_set(void *parm);
static int sq_keeper_start(void *parm);
static int sq_keeper_stop(void *parm);
static int sq_keeper_isrunning(void* parm);
static int sq_keeper_start_retry(void* parm);
static int sq_keeper_monitor(void* parm);
#define SEQ_KEEPER_WAIT_RUNNING_DELAY        5000
#define SEQ_KEEPER_MONITOR_DELAY               10
#define SEQ_KEEPER_MAX_ERROR_WAIT           10000
#define SEQ_KEEPER_READY_MODE_DEFAULT_I       900 /* .9 amps */
#define SEQ_KEEPER_READY_MODE_DEFAULT_LF    10000 /* 10%     */
#define SEQ_KEEPER_READY_MODE_DEFAULT_RETRY    20 /* 20 Retries */
/* Keeper Helper functions */
static int sq_keeper_ready_mode_default(void);

/* Anode sequence functions - declarations */
static int sq_anode_voltage_set(void *parm);
static int sq_anode_current_set(void *parm);
static int sq_anode_start(void *parm);
static int sq_anode_stop(void *parm);
static int sq_anode_isrunning(void* parm);
static int sq_anode_start_retry(void* parm);
#define SEQ_ANODE_RETRY_DELAY 2000
static int sq_anode_voltage_setpoint(void* parm);
static int sq_anode_current_setpoint(void* parm);
#define SEQ_ANODE_START_METHOD_RETRIES 3
#define SEQ_ANODE_START_V_TOLERANCE 20
static int sq_anode_method_start(void *parm);
static int sq_anode_method_ignite(void *parm);
static int sq_anode_voltage_slew_setpoint(void *parm);
static int sq_anode_monitor_stability(void *parm);

/* Magnet sequence functions - declarations */
static int sq_magnet_current_set(void *parm);
static int sq_magnet_on(void *parm);
static int sq_magnet_off(void *parm);
static int sq_magnet_current_setpoint(void *parm);
static int sq_magnet_current_check(void *parm);
static int sq_magnet_current_slew_setpoint(void *parm);

/* Utility sequence functions - declarations */
static int sq_valve_cathode_lf_set(void *parm);
static int sq_valve_cathode_hf_set(void *parm);
static int sq_valve_anode_flow_set(void *parm);
static int sq_valve_latch_valve_set(void *parm);
static int sq_valve_cathode_lf_setpoint(void *parm);
static int sq_valve_anode_flow_setpoint(void *parm);

/* Function Tables */
typedef int (*SEQUENCE_FUNCTION) (void *data);

SEQUENCE_FUNCTION utility_functions[] = {
    function_count,
    sq_util_delay,
    NULL,   // used to be sq_util_poll which was never implemented
    sq_util_set_setpoint,
    sq_util_throttle,
    sq_util_throttle_setpoint
};

#define SQ_FUNC_UTIL_COUNT (sizeof(utility_functions)/sizeof(SEQUENCE_FUNCTION))

SEQUENCE_FUNCTION error_functions[] = {
    function_count,
    sq_error_emcy_shutdown,
    sq_error_shutdown,
    sq_error_anode_shutdown,
    sq_error_mode_ready_mode_shtdn,
    sq_error_mode_steady_state_shtdn,
    sq_error_mode_throttling,
    sq_error_mode_steady_state_rm,
    sq_error_bit_shtdn,
    sq_error_conditioning_shtdn,
    sq_error_conditioning_rm
};

#define SQ_FUNC_ERROR_COUNT (sizeof(error_functions)/sizeof(SEQUENCE_FUNCTION))

SEQUENCE_FUNCTION keeper_functions[] = {
    function_count,
    sq_keeper_voltage_set,
    sq_keeper_current_set,
    sq_keeper_start,
    sq_keeper_stop,
    sq_keeper_isrunning,
    sq_keeper_start_retry,
    sq_keeper_monitor
};

#define SQ_FUNC_KEEPER_COUNT (sizeof(keeper_functions)/sizeof(SEQUENCE_FUNCTION))

SEQUENCE_FUNCTION anode_functions[] = {
    function_count,
    sq_anode_voltage_set,
    sq_anode_current_set,
    sq_anode_start,
    sq_anode_stop,
    sq_anode_isrunning,
    sq_anode_start_retry,
    sq_anode_voltage_setpoint,
    sq_anode_current_setpoint,
    sq_anode_method_start,
    sq_anode_method_ignite,
    sq_anode_voltage_slew_setpoint,
    sq_anode_monitor_stability
};
#define SQ_FUNC_ANODE_COUNT (sizeof(anode_functions)/sizeof(SEQUENCE_FUNCTION))

SEQUENCE_FUNCTION magnet_functions[] = {
    function_count,
    sq_magnet_current_set,
    sq_magnet_on,
    sq_magnet_off,
    sq_magnet_current_setpoint,
    sq_magnet_current_check,
    sq_magnet_current_slew_setpoint
};

#define SQ_FUNC_MAGNET_COUNT (sizeof(magnet_functions)/sizeof(SEQUENCE_FUNCTION))

SEQUENCE_FUNCTION valve_functions[] = {
    function_count,
    sq_valve_cathode_lf_set,
    sq_valve_cathode_hf_set,
    sq_valve_anode_flow_set,
    sq_valve_latch_valve_set,
    sq_valve_cathode_lf_setpoint,
    sq_valve_anode_flow_setpoint
};

#define SQ_FUNC_VALVE_COUNT (sizeof(valve_functions)/sizeof(SEQUENCE_FUNCTION))

SEQUENCE_FUNCTION *sequence_functions[] = {
    utility_functions,
    keeper_functions,
    anode_functions,
    magnet_functions,
    /* This slot was originally for the second magnet, but ALL magnet
     * functionality is done in one module. this is kept as a place holder
     * too keep them all aligned with the CAN IDs and the number scheme for the
     * rest of the MCUs
     */
    0,
    valve_functions,
    NULL
};

#define FUNCTION_TABLE_COUNT (sizeof(sequence_functions)/sizeof(SEQUENCE_FUNCTION *))

/* Keep these counts in the same order as the sequence functions above */
static unsigned int table_function_counts[] = {
    SQ_FUNC_UTIL_COUNT,
    SQ_FUNC_KEEPER_COUNT,
    SQ_FUNC_ANODE_COUNT,
    SQ_FUNC_MAGNET_COUNT,
    0,
    SQ_FUNC_VALVE_COUNT,
};



typedef struct abort_event {
    uint32_t event_mask;
    SEQUENCE_FUNCTION func;
} abort_event_t;

abort_event_t abort_events[ABORT_MAX_EVENTS] = {
    { 0,          sq_error_emcy_shutdown  },
    { 0x0000001E, sq_error_shutdown       },
    { 0x0000001C, sq_error_shutdown       },
    { 0x0000001A, sq_error_anode_shutdown },
    { 0x0000001E, sq_error_mode_ready_mode_shtdn},
    { 0x0000001C, sq_error_mode_ready_mode_shtdn},
    { 0x0000001E, sq_error_mode_steady_state_rm},
    { 0x0000001A, sq_error_mode_steady_state_rm},
    { 0x0000001E, sq_error_mode_throttling},
    { 0x0000001E, sq_error_bit_shtdn},
    { 0x0000001E, sq_error_conditioning_shtdn},
    { 0x0000001C, sq_error_conditioning_shtdn},
    { 0x0000001E, sq_error_conditioning_rm},
    { 0x0000001A, sq_error_conditioning_rm},
};

#define SEQUENCE_VALUE_TO_ABORT(__X__)  (abort_events[(__X__ & ABORT_MASK)])



#define PARM_TO_UINT32_VALUE(__X__) (*((uint32_t *)__X__))
#define PARM_TO_UINT16_VALUE(__X__) (*((uint16_t *)__X__))
#define PARM_TO_BOOL_VALUE(__X__)   (*((int *)__X__) ? 1:0)



////////////////////////////////////////////////////////////////////////////////////
// Private utility functions
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Count the number of functions in a table
 * @param table Index into the table_function_counts array
 * @return Number of functions
 */
static int function_count(void *table)
{
    int ret = 0;
    unsigned int table_num = *((unsigned int*)table);
    if(table_num > FUNCTION_TABLE_COUNT) {
        ret = -1;
    } else {
        ret = (int)table_function_counts[table_num];
    }
    return ret;
}

/**
 * @brief Delay for a number of milliseconds. The function divides up the delay time
 * into smaller chunks and uses vTaskDelay in a loop to delay through each chunk.
 * @param parm Delay time in milliseconds
 * @return ALways 0 for success
 */
static int sq_util_delay(void *parm)
{
    int err = 0;
    uint32_t total_delay = PARM_TO_UINT32_VALUE(parm);
    uint32_t loops = total_delay/SEQ_KEEPER_MONITOR_DELAY;
    TickType_t xNextWakeTime = xTaskGetTickCount();
    if(loops == 0) {
        loops = 1; // Minimum delay
    }
    for(uint32_t i = 0; i < loops && !abort_sequence; i++) {
        vTaskDelayUntil(&xNextWakeTime, (SEQ_KEEPER_MONITOR_DELAY/portTICK_PERIOD_MS));
    }
    return err;
}

/**
 * @brief Set a new index into the setpoint table for the new throttle command.
 * @param parm set point index at 1 (1 is the 0th row in the table)
 * @return 0 = success, non-zero otherwise
 */
static int sq_util_set_setpoint(void *parm)
{
    int err = -1;
    uint32_t setpoint = PARM_TO_UINT32_VALUE(parm);
    setpoint -= 1;
    if(setpoint < THRUST_POINTS_MAX) {
        err = 0;
        ctrl_sequence_setpoint_set(setpoint);
    }
    return err;
}

/**
 * @brief Throttle to a new setpoint passed as a parameter. This function will
 * queue a throttle sequence that will not run until the current sequence is
 * finished.
 * @param parm set point index at 1 (1 is the 0th row in the table)
 * @return 0 on success, non-zero otherwise
 */
static int sq_util_throttle(void *parm)
{
    int err = 0;
    uint32_t setpoint = PARM_TO_UINT32_VALUE(parm);
    setpoint -= 1;
    if(setpoint >= THRUST_POINTS_MAX) {
        err = __LINE__;
    }

    /* First check if we are throttling to a setpoint with anode current less than
     * 1.2A (halo 12 only?) and try to turn on the keeper if we are. If we fail to
     * turn on the keeper, we need to reset the seq_throttle_setpoint variable to the
     * current value before we error out. */
    if(!err) {
        float new_anode_i = 0;
        thrust_anode_i_setpoint_get(setpoint, &new_anode_i);
        if((new_anode_i <= ANODE_LOW_CURRENT_THRESHOLD) && !ctrl_keeper_isrunning()) {
            // Light the keeper
            uint32_t sec = 5;
            err = sq_keeper_start_retry(&sec);
        }
        else if((new_anode_i > ANODE_LOW_CURRENT_THRESHOLD) && ctrl_keeper_isrunning()) {
            // turn off the keeper
            err = sq_keeper_stop(NULL);
        }
    }

    if(!err) {
        err = ctrl_throttle_setpoint_set(setpoint);
    }
    return err;
}

/**
 * @brief Throttle to a sequence index stored in the private seq_throttle_setpoint
 * variable. This function will queue a throttle sequence that will not run until
 * the current sequence is finished.
 * @param parm not used
 * @return 0 on success, non-zero otherwise
 */
static int sq_util_throttle_setpoint(void *parm)
{
    int err = 0;

    /* First check if we are throttling to a setpoint with anode current less than
     * 1.2A (halo 12 only?) and try to turn on the keeper if we are. If we fail to
     * turn on the keeper, we need to reset the seq_throttle_setpoint variable to the
     * current value before we error out. */
    float new_anode_i = 0;
    thrust_anode_i_setpoint_get(seq_throttle_setpoint, &new_anode_i);
    if((new_anode_i <= ANODE_LOW_CURRENT_THRESHOLD) && !ctrl_keeper_isrunning()) {
        // Light the keeper
        uint32_t sec = 5;
        err = sq_keeper_start_retry(&sec);
        if(err) {
            // reset the setpoint to current before erroring out
            seq_throttle_setpoint = seq_throttle_setpoint_prev;
        }
    }
    else if((new_anode_i > ANODE_LOW_CURRENT_THRESHOLD) && ctrl_keeper_isrunning()) {
        // turn off the keeper
        err = sq_keeper_stop(NULL);
    }

    if(!err) {
        err = ctrl_throttle_setpoint_set(seq_throttle_setpoint);
    }

    return err;
}



////////////////////////////////////////////////////////////////////////////////////
// Private sequence error functions
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Shut down the thruster when an error occurs transitioning through the
 * Steady State sequence.
 * @param parm Not used
 * @return Always return success
 */
static int sq_error_mode_steady_state_shtdn(void* parm)
{
    int err = 0;
    err = sq_error_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_STEADY_STATE_FAULT, &sq);

    return err;
}

/**
 * @brief Return to Ready Mode when an error occurs transitioning through the Steady
 * State sequence. Only used for Halo 6.
 * @param parm Not used
 * @return 0 = success, non-zero otherwise
 */
static int sq_error_mode_steady_state_rm(void* parm)
{
    int err = 0;
    err = sq_error_anode_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_STEADY_STATE_FAULT, &sq);
    
    return err;
}

/**
 * @brief Shut down the thruster when an error occurs transitioning through the
 * Ready Mode sequence.
 * @param parm
 * @return Always return success.
 */
static int sq_error_mode_ready_mode_shtdn(void* parm)
{
    int err = 0;
    err = sq_error_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_READY_MODE_FAULT, &sq);

    return err;
}

/**
 * @brief Shutdown the thruster when an error occurs transitioning through the
 * throttling sequence.
 * @param parm  Not used
 * @return Always return success.
 */
static int sq_error_mode_throttling(void* parm)
{
    int err = 0;
    err = sq_error_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_THROTTLING_FAULT, &sq);

    return err;
}

/**
 * @brief Shutdown the thruster when an error occurs transitioning through the
 * BIT sequence.
 * @param parm  Not used
 * @return Always return success.
 */
static int sq_error_bit_shtdn(void* parm)
{
    int err = 0;
    err = sq_error_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_BIT_FAULT, &sq);

    return err;
}

/**
 * @brief Shutdown the thruster when an error occurs transitioning through the
 * conditioning sequence. Only used for Halo 6.
 * @param parm  Not used
 * @return Always return success.
 */
static int sq_error_conditioning_shtdn(void* parm)
{
    int err = 0;
    err = sq_error_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_CONDITIONING_FAULT, &sq);

    return err;
}

/**
 * @brief Return to Ready Mode when an error occurs transitioning through the
 * conditioning sequence. Why??? Only used on Halo 6.
 * @param parm  Not used
 * @return Always return success.
 */
static int sq_error_conditioning_rm(void* parm)
{
    int err = 0;
    err = sq_error_anode_shutdown(NULL);
    sq_control_specific_detail_t sq = {.sq_err = err};
    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_CONDITIONING_FAULT, &sq);

    return err;
}

/**
 * @brief Power down MCU's
 * @param parm
 * @return Always returns success
 */
static int sq_error_emcy_shutdown(void *parm)
{
    int err = 0;
    client_emcy_shutdown();
    return err;
}

/**
 * @brief Turn off the Magnets, Anode, and Keeper.  Then set all MCU's to defaults.
 * @param parm Not used
 * @return 0 on success, non-zero otherwise
 */
int sq_error_shutdown(void *parm)
{
    int err = 0;

    cmd_anode_ps_state_set(COMMANDED_OFF);
    cmd_magnet_state_set(COMMANDED_OFF);
    cmd_keeper_ps_state_set(COMMANDED_OFF);
    ctrl_valves_reinit();
    ctrl_magnet_reinit();
    cmd_anode_reinit();
    cmd_keeper_reinit();
    
    return err;
}

/**
 * @brief Turn off the Magnets, Anode, and Anode flow.  Set keeper to default (if it
 * is on). Not used for Halo 12, it cannot shutdown the anode without also shutting
 * down the keeper.
 * @param parm Not used
 * @return 0 on success, non-zero otherwise
 */
int sq_error_anode_shutdown(void *parm)
{
    int err = 0;

    err = cmd_anode_ps_state_set(COMMANDED_OFF);
    err = cmd_magnet_state_set(COMMANDED_OFF);
    
    err = ctrl_magnet_reinit();
    err = cmd_anode_reinit();
    
    err = ctrl_valves_anode_flow_set((float)0.0);
    err = sq_keeper_ready_mode_default();
    
    return err;
}



////////////////////////////////////////////////////////////////////////////////////
// Private Keeper Sequence Functions
////////////////////////////////////////////////////////////////////////////////////

/* Most Float Values are multiplied by 1000 so they can be packed into 
 * the 24 BIT field in the sequence parameter - Define any special cases here */
#define KEEPER_PARM_FACTOR_VOLTS     SEQ_PARM_FACTOR_FLOAT
#define KEEPER_PARM_FACTOR_CURRENT   SEQ_PARM_FACTOR_FLOAT

#define KEEPER_PARM_CONVERT_VOLTS(__X__) \
    (float_t) ((float_t)__X__ / (float_t)KEEPER_PARM_FACTOR_VOLTS)
#define KEEPER_PARM_CONVERT_CURRENT(__X__) \
    (float_t) ((float_t)__X__ / (float_t)KEEPER_PARM_FACTOR_CURRENT)

/**
 * @brief Set the anode voltage.
 * @param parm anode voltage multiplied by 1000 (150.5v = 150500)
 * @return 0 on success, non-zero otherwise
 */
static int sq_keeper_voltage_set(void *parm)
{
    int err = 0;
    uint32_t value = PARM_TO_UINT32_VALUE(parm);
    float_t volts = KEEPER_PARM_CONVERT_VOLTS(value);
    err = cmd_keeper_volts_set(volts);
    return err;
}

/**
 * @brief Set the keeper current
 * @param parm keeper current multiplied by 1000 (2.5a = 2500)
 * @return 0 on success, non-zero otherwise
 */
static int sq_keeper_current_set(void *parm)
{
    int err = 0;
    uint32_t value = PARM_TO_UINT32_VALUE(parm);
    float_t current = KEEPER_PARM_CONVERT_CURRENT(value);
    err = cmd_keeper_cur_set(current);
    return err;
}

/**
 * @brief Check to make sure the keeper is running
 * @param param not used
 * @return 0 if keeper is running, non-zero otherwise
 */
static int sq_keeper_isrunning(void *parm)
{
    int err = 0;
    int running = ctrl_keeper_isrunning();
    if(!running) { // Keeper is not running
        err = __LINE__;
    }
    return err;
}

/**
 * @brief Start the keeper. Only used for Halo 6. Halo 12 calls sq_anode_method_ignite
 * @param timeout waiting for response from Keeper
 * @return zero on success, non-zero otherwise
 */
static int sq_keeper_start(void *parm)
{
    int err = 0;
    uint32_t timeout = PARM_TO_UINT32_VALUE(parm);
    err = cmd_keeper_start(timeout);
    if(!err) {
        err = sq_keeper_isrunning(NULL);
        if(err) {
            /* It went out - just wait here for it to clear */
            ctrl_keeper_wait_err_clear(SEQ_KEEPER_MAX_ERROR_WAIT);
        }
    }
    TraceDbg(TrcMsgSeq, "err:%d", err, 0,0,0,0,0);
    return err;
}

/**
 * @brief Start the keeper with some number of retries. Only used for Halo 6. Halo 12
 * calles sq_anode_method_ignite.
 * @param parm Number of seconds to try
 * @return zero on success, non-zero otherwise
 */
static int sq_keeper_start_retry(void *parm)
{
    int err = 0;
    int start_err = __LINE__; // start returns 0 on success
    int retry_count = 1;
    (void)retry_count;
        
    uint32_t to = SEQ_DEFAULT_TO;
    uint32_t retry_seconds = PARM_TO_UINT32_VALUE(parm);
    
    static SYSTMR retry_timer = 0;
    sys_timer_start(retry_seconds SECONDS, &retry_timer);
    TraceDbg(TrcMsgSeq, "abort:%08x t:%d", abort_sequence , retry_timer,0,0,0,0);
    
    while(retry_timer > 0 && start_err && !abort_sequence) {
        TraceDbg(TrcMsgSeq, "Attempt:%d", retry_count,0,0,0,0,0);
        start_err = sq_keeper_start(&to);
        if(!start_err) {
            uint32_t monitor_to = SEQ_KEEPER_WAIT_RUNNING_DELAY;
            start_err = sq_keeper_monitor(&monitor_to);
        }
        TraceDbg(TrcMsgSeq, "Attempt:%d start_err:%d", retry_count++,
                start_err,0,0,0,0);
    }
    if(start_err) {
        err = start_err;
    }
    sys_timer_abort(&retry_timer);
    return err;
}

/**
 * @brief Monitor the keeper for the specified time.May only be needed for Halo 6.
 * @param parm milliseconds to monitor the keeper is running normally
 * @return 0 on success (keeper still running), non-zero otherwise
 */
static int sq_keeper_monitor(void *parm)
{
    int err = 0;
    uint32_t total_delay = PARM_TO_UINT32_VALUE(parm);
    uint32_t loops = total_delay/SEQ_KEEPER_MONITOR_DELAY;
    
    TickType_t xNextWakeTime = xTaskGetTickCount();
    
    if(loops == 0) {
        loops = 1; // Minimum delay
    }
    uint32_t i = 0;
    for(; i < loops && !err && !abort_sequence; i++) {
        vTaskDelayUntil(&xNextWakeTime, (SEQ_KEEPER_MONITOR_DELAY/portTICK_PERIOD_MS));
        err = sq_keeper_isrunning(NULL);
    }
    if(err) {
       ctrl_keeper_wait_err_clear(SEQ_KEEPER_MAX_ERROR_WAIT);
    }
    TraceDbg(TrcMsgSeq, "monitor done i:%d running:%d", i, err, 0,0,0,0);
    return err;
}
/**
 * @brief Turn Keeper Power off.
 * @param parm not used
 * @return 0 on success, non-zero otherwise
 */
static int sq_keeper_stop(void *parm)
{
    int err = 0;
    err = cmd_keeper_ps_state_set(COMMANDED_OFF);
    return err;
}

/**
 * @brief Return the keeper to its default settings
 * @return 0 on success, non-zero otherwise
 */
static int sq_keeper_ready_mode_default(void)
{
    int valve_err = 0;
    int keeper_err = 0;
    
    uint32_t lf_parm = 0;
    uint32_t i_parm = DEFAULT_KEEPER_CURRENT * 1000;
    
    int is_running = ctrl_keeper_isrunning();
    if (is_running) {
        lf_parm = SEQ_KEEPER_READY_MODE_DEFAULT_LF;
        i_parm = SEQ_KEEPER_READY_MODE_DEFAULT_I;
    }
    
    valve_err = sq_valve_cathode_lf_set(&lf_parm);
    keeper_err = sq_keeper_current_set(&i_parm);
    
    return (valve_err << 16) | (keeper_err & 0xFFFF);
}



////////////////////////////////////////////////////////////////////////////////////
// Private Anode Sequence Functions common to both Halo 6 and 12
////////////////////////////////////////////////////////////////////////////////////

/* Most Float Values are multiplied by 1000 so they can be packed into 
 * the 24 BIT field in the sequence parameter - Define any special cases here */
#define ANODE_PARM_FACTOR_VOLTS     SEQ_PARM_FACTOR_FLOAT
#define ANODE_PARM_FACTOR_CURRENT   SEQ_PARM_FACTOR_FLOAT
#define ANODE_VOLTAGE_SLEW_STEP        25                     // Number of V/s to slew

#define ANODE_PARM_CONVERT_VOLTS(__X__) \
    (float_t) ((float_t)__X__ / (float_t)ANODE_PARM_FACTOR_VOLTS)
#define ANODE_PARM_CONVERT_CURRENT(__X__) \
    (float_t) ((float_t)__X__ / (float_t)ANODE_PARM_FACTOR_CURRENT)

/**
 * @brief Manually set the anode voltage
 * @param parm anode voltage multiplied by 1000 (150.5v = 150500)
 * @return 0 on success, non-zero otherwise
 */
static int sq_anode_voltage_set(void *parm)
{
    int err = 0;
    uint32_t value = PARM_TO_UINT32_VALUE(parm);
    float_t volts = ANODE_PARM_CONVERT_VOLTS(value);
    err = cmd_anode_volts_set(volts);
    return err;
}

/**
 * @brief Set the anode voltage to the current voltage setpoint
 * @param parm unused
 * @return 0 on success, non-zero otherwise
 */
static int sq_anode_voltage_setpoint(void *parm)
{
    int err = 0;
    float_t volts = 0;
    err = thrust_anode_v_setpoint_get(seq_throttle_setpoint, &volts);
    if(!err) {
        err = cmd_anode_volts_set(volts);
    }
    return err;
}

/**
 * @brief Manually set the anode current.
 * @param parm anode current multiplied by 1000 (2.5a = 2500)
 * @return 0 on success, non-zero otherwise
 */
static int sq_anode_current_set(void *parm)
{
    int err = 0;
    uint32_t value = PARM_TO_UINT32_VALUE(parm);
    float_t current = ANODE_PARM_CONVERT_CURRENT(value);
    err = cmd_anode_cur_set(current);
    return err;
}

/**
 * @brief Set the anode current to the current setpoint (isn't the English language fun?)
 * @param parm unused
 * @return 0 on success, non-zero otherwise
 */
static int sq_anode_current_setpoint(void *parm)
{
    int err = 0;
    float_t current = 0;
    err = thrust_anode_i_setpoint_get(seq_throttle_setpoint, &current);
    if(!err) {
        err = cmd_anode_cur_set(current);
    }
    return err;
}

/**
 * @brief Check to make sure the anode is running
 * @param parm not used
 * @return 0 if anode is running, non-zero otherwise
 */
static int sq_anode_isrunning(void *parm)
{
    int err = 0;
    int running = ctrl_anode_isrunning();
    if(!running) { // anode is not running
        err = __LINE__;
    }
    return err;
}

/**
 * @brief Turn the anode power off
 * @param parm not used
 * @return 0 on success, non-zero otherwise
 */
static int sq_anode_stop(void *parm)
{
    int err = 0;
    err = cmd_anode_ps_state_set(COMMANDED_OFF);
    return err;
}

/**
 * @brief Monitor anode current stability by confirming actual current is in range
 * of expected for three consecutive measurements.
 * @param parm 0 = use current setpoint value, otherwise this is milliamps
 * @return 0 = current stable, non-zero if we fail to get three consecutive
 * measurements within sixty seconds.
 */
static int sq_anode_monitor_stability(void *parm)
{
    int err = 0;
    uint32_t target = PARM_TO_UINT32_VALUE(parm);
    float_t ftarget;

    if(target == 0) {
        thrust_anode_i_setpoint_get(seq_throttle_setpoint, &ftarget);
    }
    else {
        ftarget = ANODE_PARM_CONVERT_CURRENT(target);
    }

    err = cmd_anode_monitor_stability(ftarget);
    return err;
}


////////////////////////////////////////////////////////////////////////////////////
// Private Anode Sequence Functions specific to Halo 6
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Start the anode. Halo 6 only.
 * @param timeout waiting for response from Anode
 * @return zero on success, non-zero otherwise
 */
static int sq_anode_start(void *parm)
{
    int err = 0;
    uint32_t timeout = PARM_TO_UINT32_VALUE(parm);
    err = ctrl_anode_start(timeout);
    if(!err) {
        err = sq_anode_isrunning(NULL);
    }
    return err;
}

/**
 * @brief Start the anode with some number of retries. Halo 6 only.
 * @param parm number of retries
 * @return zero on success, non-zero otherwise
 */
static int sq_anode_start_retry(void *parm)
{
    int err = 0;
    int start_err = __LINE__; // start returns 0 on success

    uint32_t to = SEQ_DEFAULT_TO;
    uint32_t retries = PARM_TO_UINT32_VALUE(parm);

    for(uint32_t i = 0; i < retries && start_err && !abort_sequence; i++) {
        start_err = sq_anode_start(&to);
        if(!start_err) {
            /* Delay and then make sure it is still running */
            SEQUENCE_MS_DELAY(500); // @fixme define that delay
            start_err = sq_anode_isrunning(NULL);
        }
        if(start_err) {
            SEQUENCE_MS_DELAY(SEQ_ANODE_RETRY_DELAY);
        }
    }
    if(start_err) {
        err = start_err;
    }
    return err;
}

/**
 * @brief Anode start up method for Halo 6. This will either be a HARD or GLOW start
 * depending on the current setpoint.
 * @param parm not used - This is a setpoint function
 * @return 0 = success, otherwise non-zero
 */
static int sq_anode_method_start(void *parm)
{
    int err = 0;
    int anode_started = 0;
    
    uint32_t retries = PARM_TO_UINT32_VALUE(parm);
    uint32_t hf_setpoint = 0;
    
    err = thrust_hf_start_setpoint_get(seq_throttle_setpoint, &hf_setpoint);
    TraceInfo(TrcMsgSeq, "r:%d e:%d s:%d", retries, err, hf_setpoint,0,0,0);
    for(uint32_t i = 0; i < retries && !anode_started && !abort_sequence && !err; i++) {
        err = sq_valve_cathode_hf_set(&hf_setpoint);
        if(!err) {
            SEQUENCE_MS_DELAY(500);
            uint32_t to = SEQ_DEFAULT_TO;
            err = sq_anode_start(&to);
            if(!err) { anode_started = 1; }
            //anode_started = err ? 0 : 1;
        }

        /* Turn off flow in any case */
        uint32_t sp = 0;
        err = sq_valve_cathode_hf_set(&sp);

        if(!err) {
            SEQUENCE_MS_DELAY(5000);
            if(anode_started) {
                /* Make sure its still started */
                err = sq_anode_isrunning(0);
                /* is running returns err if it is not running*/
                if(err) { anode_started = 0; }
                //anode_started = err ? 0:1;
            }
        }

        TraceInfo(TrcMsgSeq, "e:%d s:%d", err, anode_started,0,0,0,0);
    }
    if(!anode_started) {
        err = __LINE__;
    }
    return err;
}

////////////////////////////////////////////////////////////////////////////////////
// Private Anode Sequence Functions specific to Halo 12
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Starts the ignition process for Halo 12. The ignition setpoints for valves, keeper,
 * magnets and anode should already have been set in the ready mode sequence and the magnet
 * power supply is on. Now we need to turn on the anode and keeper and wait for a spark.
 * @param   parm number of retries before sending error
 * @return  0 for success, otherwise an appropriate error code
 */
static int sq_anode_method_ignite(void *parm)
{
    int err = 0;
    bool anode_started = false;

    uint32_t retries = PARM_TO_UINT32_VALUE(parm);
    TraceInfo(TrcMsgSeq, "r:%d e:%d", retries, err, 0,0,0,0);

    for(uint32_t i = 0; i < retries && !anode_started && !abort_sequence; i++)
    {
        /* Turn on the anode power supply but do not wait for spark. We will rely on
         * the anode_control.status.reason being set to SPARK_DETECTED by the
         * state_bcast_callback. */
        err = ctrl_anode_start(0);

        if(!err) {
            /* Start the keeper and wait for a spark. Give it 4750ms to fire. HW ctrl
             * hard codes 5s to fail but cmd_keeper_start will return success if we
             * wait that long. */
            err = ctrl_keeper_start(4750);
        }

        if(!err) {
            SEQUENCE_MS_DELAY(1000);  // delay to allow anode state change to spark detected
            err = sq_anode_isrunning(NULL);
        }

        if(!err) {
            // anode sparked, delay and check again to see if stayed lit
            SEQUENCE_MS_DELAY(5000);
            err = sq_anode_isrunning(NULL);
        }

        if(!err) {
            anode_started = true;
        }
        else {
            ctrl_anode_ps_state_set(COMMANDED_OFF);
            ctrl_keeper_ps_state_set(COMMANDED_OFF);
            SEQUENCE_MS_DELAY(3000);  // delay before looping back and starting again
        }
        TraceInfo(TrcMsgSeq, "e:%d s:%d", err, (uint8_t)anode_started,0,0,0,0);
    }

    if(!anode_started) {
        err = __LINE__;
    }

    return err;
}

/**
 * @brief Slew the anode voltage to a new setpoint value. Only required for Halo 12.
 * @param parm Not used.
 * @return 0 = Success, otherwise line number for the error.
 */
static int sq_anode_voltage_slew_setpoint(void *parm)
{
    int err = 0;

    float fnew_voltage = 0;
    err = thrust_anode_v_setpoint_get(seq_throttle_setpoint, &fnew_voltage);

    float ftemp_voltage = cmd_anode_volts_get();
    double delta = 0;
    float sign = (fnew_voltage < ftemp_voltage) ? (float)-1.0 : (float)1.0;

    while(ftemp_voltage != fnew_voltage)
    {
        delta = fabs((double)(fnew_voltage - ftemp_voltage));
        if(delta <= ANODE_VOLTAGE_SLEW_STEP) {
            ftemp_voltage = fnew_voltage;
        }
        else {
            ftemp_voltage += sign * (float)ANODE_VOLTAGE_SLEW_STEP;
        }

        err = cmd_anode_volts_set(ftemp_voltage);
        if(err) {
            break;
        }

        vTaskDelay(1000/portTICK_RATE_MS);
    }

    return err;
}



////////////////////////////////////////////////////////////////////////////////////
// Private Magnet Sequence Functions
////////////////////////////////////////////////////////////////////////////////////
#define MAGNET_PARM_FACTOR_RATIO        SEQ_PARM_FACTOR_FLOAT
#define MAGNET_PARM_FACTOR_CURRENT      SEQ_PARM_FACTOR_FLOAT
#define MAGNET_CURRENT_SLEW_STEP        0.25                     // Number of A/s to slew

#define MAGNET_PARM_CONVERT_RATIO(__X__) \
    (float_t) ((float_t)__X__ / (float_t)MAGNET_PARM_FACTOR_RATIO)
#define MAGNET_PARM_CONVERT_CURRENT(__X__) \
    (float_t) ((float_t)__X__ / (float_t)MAGNET_PARM_FACTOR_CURRENT)

/**
 * @brief Set the magnet current.
 * @param parm 24 bit integer
 * @return 0 = success, non-zero otherwise
 */
static int sq_magnet_current_set(void *parm)
{
    int err = 0;
    uint32_t value = PARM_TO_UINT32_VALUE(parm);
    float fvalue = MAGNET_PARM_CONVERT_CURRENT(value);
    err = cmd_magnet_current_set(fvalue);
    return err;
}

/**
 * @brief Turn on the magnets
 * @param parm not used
 * @return 0 on success, non-zero otherwise
 */
static int sq_magnet_on(void *parm)
{
    int err = 0;
    err = cmd_magnet_state_set(COMMANDED_ON);
    return err;
}
/**
 * @brief Turn magnets off by zeroing current and ratio
 * @param parm not used
 * @return 0 = success, non-zero otherwise
 */
static int sq_magnet_off(void *parm)
{
    int err = 0;
    // Shut them both off - ignoring errors between the two
    int temp_err = 0;

    temp_err = cmd_magnet_state_set(COMMANDED_OFF);
    if(temp_err) {
        err = -1;
    }

    temp_err = cmd_magnet_current_set(0);
    if( temp_err ) {
        err = -1;
    }

    return err;
}

/**
 * @brief Set the magnet current to the current throttle setpoint.
 * @param parm 24 bit integer
 * @return 0 = success, non-zero otherwise
 */
static int sq_magnet_current_setpoint(void *parm)
{
    int err = 0;
    float fvalue = 0;
    err = thrust_magnet_i_setpoint_get(seq_throttle_setpoint, &fvalue);
    if(!err) {
        err = cmd_magnet_current_set(fvalue);
    }
    return err;
}

/**
 * @brief Check to make sure magnet current is in an acceptable range according to the
 * current set point.
 * @param parm Not used
 * @return 0 = success, non-zero otherwise
 */
static int sq_magnet_current_check(void *parm)
{
    int err = 0;
    err = cmd_magnet_current_check();
    return err;
}

/**
 * @brief Slew the magnet current to a new setpoint value. Only required for Halo 12.
 * @param parm Not used.
 * @return 0 = Success, otherwise line number for the error.
 */
static int sq_magnet_current_slew_setpoint(void *parm)
{
    int err = 0;

    float fnew_current = 0;
    err = thrust_magnet_i_setpoint_get(seq_throttle_setpoint, &fnew_current);

    float ftemp_current = cmd_magnet_current_get();
    double delta = 0;
    float sign = (fnew_current < ftemp_current) ? (float)-1.0 : (float)1.0;

    while(ftemp_current != fnew_current)
    {
        delta = fabs((double)(fnew_current - ftemp_current));
        if(delta <= MAGNET_CURRENT_SLEW_STEP) {
            ftemp_current = fnew_current;
        }
        else {
            ftemp_current += sign * (float)MAGNET_CURRENT_SLEW_STEP;
        }

        err = cmd_magnet_current_set(ftemp_current);
        if(err) {
            break;
        }

        vTaskDelay(1000/portTICK_RATE_MS);
    }

    return err;
}



////////////////////////////////////////////////////////////////////////////////////
// Private Valve Sequence Functions
////////////////////////////////////////////////////////////////////////////////////

#define VALVE_PARM_FACTOR_PSI     SEQ_PARM_FACTOR_FLOAT

#define VALVE_PARM_CONVERT_PSI(__X__) \
    (float_t) ((float_t)__X__ / (float_t)VALVE_PARM_FACTOR_PSI)
/**
 * @brief Manually set the cathode low flow psi.
 * @param parm 16 bit psi integer
 * @return 0 on success or -1 on failure
 */
static int sq_valve_cathode_lf_set(void *parm)
{
    int err = 0;
    uint32_t psi = PARM_TO_UINT16_VALUE(parm);
    float fpsi = VALVE_PARM_CONVERT_PSI(psi);
    err = ctrl_valves_cathode_lf_set(fpsi);
    return err;
}

/**
 * @brief Set the cathode low flow psi to the current throttle setpoint
 * @param parm not used
 * @return 0 on success or -1 on failure
 */
static int sq_valve_cathode_lf_setpoint(void *parm)
{
    int err = 0;
    float fpsi = 0;
    err = thrust_cathode_lf_setpoint_get(seq_throttle_setpoint, &fpsi);
    if(!err) {
        err = ctrl_valves_cathode_lf_set(fpsi);
    }
    return err;
}

/**
 * @brief Manually set the cathode high flow psi. The high flow is generally set to
 * on (99% duty cycle) or off (0%) so a setpoint function is not required.
 * @param parm 16 bit psi integer
 * @return 0 on success or -1 on failure
 */
int sq_valve_cathode_hf_set(void *parm)
{
    int err = 0;
    uint32_t psi = PARM_TO_UINT32_VALUE(parm);
    float fpsi = VALVE_PARM_CONVERT_PSI(psi);
    err = ctrl_valves_cathode_hf_set(fpsi);
    return err;
}

/**
 * @brief Manually set the anode flow psi (overrides the current setpoint value)
 * @param parm 16 bit psi integer
 * @return 0 on success or -1 on failure
 */
int sq_valve_anode_flow_set(void *parm)
{
    int err = 0;
    uint32_t psi = PARM_TO_UINT32_VALUE(parm);
    float fpsi = VALVE_PARM_CONVERT_PSI(psi);
    err = ctrl_valves_anode_flow_set(fpsi);
    return err;
}

/**
 * @brief Set the anode flow psi to the current throttle setpoint
 * @param parm not used
 * @return 0 on success or -1 on failure
 */
static int sq_valve_anode_flow_setpoint(void *parm)
{
    int err = 0;
    float fpsi = 0;
    err = thrust_anode_flow_setpoint_get(seq_throttle_setpoint, &fpsi);
    if(!err) {
        err = ctrl_valves_anode_flow_set(fpsi);
    }
    return err;
}

/**
 * @brief Open/Close the latch valve
 * @param parm boolean, non-zero = on, zero = off
 * @return 0 on success or -1 on failure
 */
static int sq_valve_latch_valve_set(void *parm)
{
    int err = 0;
    bool state = PARM_TO_BOOL_VALUE(parm);
    err = ctrl_valves_latch_valve_set(state ? ON_SET_POINT:OFF_SET_POINT);
    return err;
}



////////////////////////////////////////////////////////////////////////////////////
// Private Sequence Execution Functions
////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Sequence runner - this is what runs the actual sequence
 * @param sequence pointer to sequence
 * @return 0 = success, non-zero otherwise.  Error will be set in the pointer to
 * sequence structure status field (the sequence pointer passed to this function)
 */
static int ctrl_sequence_exe(sequence_run_t *sequence)
{
    int err = 0;
    bool eol =  false;
    sequence_array_t *pSeq = sequence->seq;

    /* Mark sequence as running */
    SEQUENCE_STATUS_SET(sequence->status, SEQ_STAT_RUNNING);
    
    for(uint32_t i = 0; i < sequence->size && !err && !eol; i++) {
        if(!pSeq || pSeq[i] == 0) { // 0 is end of list
            eol = true;
            if(!pSeq) {
                TraceDbg(TrcMsgDbg, "pSeq is null:%p", pSeq, 0,0,0,0,0);
            }
        } else {
            /* table, functions, error callback, and abort are in the upper 32*/
            uint32_t seq_step_info = SEQUENCE_TO_FUNCTION_PARAMETERS(pSeq[i]);
            uint32_t table = SEQUENCE_VALUE_TO_TABLE_NUM(seq_step_info);
            uint32_t function = SEQUENCE_VALUE_TO_FUNC_NUM(seq_step_info);
            uint32_t error_func = SEQUENCE_VALUE_TO_ERR_NUM(seq_step_info);
            
            /* Set the status */
            SEQUENCE_VALUE_TO_STATUS(sequence->status, seq_step_info);
            
            /* Update the current abort code, which is stored in this module's
             * global area so that external modules can notify the current
             * running sequence of a potential abort event
             */
            abort_event_t abort_event = SEQUENCE_VALUE_TO_ABORT(seq_step_info);
            abort_event_mask = abort_event.event_mask;
            
            /* Parameters are in the lower 32 */
            uint32_t parm = SEQUENCE_VALUE_TO_PARM(pSeq[i]);
            
            /* Error check sequence values */
            if(table >= FUNCTION_TABLE_COUNT) {
                err = __LINE__;
            }
            if(!err  && function >= table_function_counts[table]) {
                err = __LINE__;
            }
            if(!err && error_func > SQ_FUNC_ERROR_COUNT) {
                err = __LINE__;
            }
            if(!err && abort_event.func == 0) {
                err = __LINE__;
            }
            
            if(err) {
                SEQUENCE_STATUS_SET(sequence->status, SEQ_STAT_ERROR);
                SEQUENCE_ERROR_SET(sequence->status, err);
            }
            
            /* Execute the Sequence Step */
            if(!err) {
                TraceDbg(TrcMsgSeq,"t:%x f:%x p:%x", table, function, parm,0,0,0);
                SEQUENCE_FUNCTION func = sequence_functions[table][function];
                err = func((void*)&parm);
                if(err) {
                    SEQUENCE_STATUS_SET(sequence->status, SEQ_STAT_ERROR);
                    SEQUENCE_ERROR_SET(sequence->status, err);
                    /* Time to run the error function */
                    TraceE3(TrcMsgSeq,
                       "ERROR Seq:%s step:%d table:%d func:%d parm:%x error:%d",
                       (int)sequence->name, i, table, function, parm, error_func);
                    SEQUENCE_FUNCTION err_func = error_functions[error_func];
                    /* Error functions must not fail! Or at least if they
                     * succeed, do not reset 'err', else the sequence will
                     * continue to run
                     */
                    err_func(NULL);
                }
            }
            // ToDo: clion says this has been optimized out but it worked on Halo 6. Watch this during testing.
            if(abort_sequence) {
                if(!err) {
                    /* If there was no error, then the sequence was aborted,
                     * else if there was an error leave the error alone
                     */
                    SEQUENCE_STATUS_SET(sequence->status, SEQ_STAT_ABORTED);
                    SEQUENCE_ERROR_SET(sequence->status, abort_sequence);
                }
                /* call the abort event function */
                abort_event.func(NULL);
                err = __LINE__; // aborted
                TraceDbg(TrcMsgSeq,"aborted:0x%08x", abort_sequence,0,0,0,0,0);
            }
            if(err) {
                /* If there was any error at all, empty any outstanding
                 * sequences.  Its up to the caller to make decisions about
                 * what sequence should be called next if there was an error
                 * or an abort. Anything that is queued should only be called
                 * if its predecessors all passed. 
                 */
                while( uxQueueMessagesWaiting(xSeqQueue) ) {
                    sequence_run_t abort_seq;
                    BaseType_t xStatus = xQueueReceive(xSeqQueue, &abort_seq, 0);
                    if(xStatus == pdTRUE) {
                        SEQUENCE_STATUS_SET(abort_seq.status, SEQ_STAT_ABORTED);
                        SEQUENCE_ERROR_SET(abort_seq.status, abort_sequence);
                    }
                }
            }
        }
    }
    /* Clear abort code, sequence no longer running */
    abort_event_mask = 0;
    if(!err) {
        SEQUENCE_STATUS_SET(sequence->status, SEQ_STAT_SUCCESS);
    }
    return err;
}
/**
 * @brief Launch the sequence engine task. It waits for a sequence to be added to
 * a queue and then executes the sequence.
 * @param pvParameters Unused. The sequence to be run is set by calling ctrl_sequence_run
 */
_Noreturn static void ctrl_sequence_task(void *pvParameters)
{
    while(1) {
        BaseType_t xStatus = xQueueReceive(xSeqQueue, &seq, portMAX_DELAY);
        if(xStatus == pdTRUE) {
            /* Start with a clean abort slate */
            abort_sequence = 0;
            ctrl_sequence_exe(&seq);
            if(seq.psem) {
                /* If the sequence has a semaphore pointer, there is a task
                 * blocking and waiting for it to finish */
                xSemaphoreGive(*(seq.psem));
            }
        }
    }
}

/**
 * @brief Create a task for the sequence engine.
 */
static void ctrl_sequence_start_task(void)
{
    xTaskCreateStatic(ctrl_sequence_task, "Sequence Task", seq_TASK_STACK_SIZE,
            NULL, seq_TASK_PRIORITY, seq_task_stack, &seq_TaskBuffer);
}



////////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////////

uint32_t ctrl_sequence_setpoint_get()
{
    return seq_throttle_setpoint;
}

void ctrl_sequence_setpoint_set(uint32_t sp)
{
    seq_throttle_setpoint_prev = seq_throttle_setpoint;
    seq_throttle_setpoint = sp;
}

uint32_t ctrl_sequence_status_get(void)
{
    return *seq.status;
}

void ctrl_sequence_abort(void)
{
    TraceE3(TrcMsgSeq,"Force Abort",0,0,0,0,0,0);
    abort_sequence = ~0;
}

int ctrl_sequence_abort_error(sequence_abort_error_t error)
{
    uint32_t abort_error = 1 << error;
    uint32_t abort = abort_error & abort_event_mask;
    if(abort) {
        TraceE3(TrcMsgSeq,"Sequence aborted for error:%x abort mask:%08x",
                error, abort_event_mask,0,0,0,0);
        abort_sequence = error;
    }

    return (int)abort;
}

int ctrl_sequence_error_shutdown(void)
{
    return sq_error_shutdown(NULL);
}

int ctrl_sequence_standby_return(void)
{
    int err = 0;
    //ToDo: shut down the anode, magnets, and keeper and return to the standby state
    err = sq_error_shutdown(NULL);

    return err;
}

int ctrl_sequence_run(sequence_run_t *pSeq)
{
    if(*pSeq->status == SEQ_STAT_QUEUED ||
       *pSeq->status == SEQ_STAT_RUNNING ) {
        return -1;
    }
    SEQUENCE_STATUS_SET(pSeq->status, SEQ_STAT_QUEUED);
    xQueueSend(xSeqQueue, pSeq, 0);
    return 0;
}

int ctrl_sequence_throttle_run(sequence_run_t *pSeq, uint32_t setpoint)
{
    ctrl_sequence_setpoint_set(setpoint);
    return ctrl_sequence_run(pSeq);
}

void sq_control_error_handler_init(void)
{
    fh_fault_handlers_register(ERROR_CODE_SQNC_READY_MODE_FAULT,   FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_SQNC_STEADY_STATE_FAULT, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_SQNC_THROTTLING_FAULT,   FH_ALERT);
}

void ctrl_sequence_init(void)
{
    xSeqQueue = xQueueCreateStatic( BUFFER_COUNT_SEQ, QUEUE_ITEM_SIZE_SEQ,
                                   ucSeqQueueStorageArea, &xSeqStaticQueue );
    ctrl_sequence_start_task();
}



/* *****************************************************************************
 End of File
 */
