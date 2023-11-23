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
 * File:   control_throttle.c
 * Author: jmeyers
 * 
 * 
 * This file contains all the logic an routine to perform throttling from
 * one setpoint to another setpoint
 * 
 *
 * Created on August 9, 2021, 7:14 AM
 */

#include <memory.h>
#include "control_throttle.h"             // This header
#include "anode/control_anode.h"          // Anode conversion macros
#include "client-control/client_control.h"// Update state
#include "sequence_throttle.h"
#include "setpoint/control_setpoint.h"// Truster tables
#include "trace/trace.h"              // Trace messages
#include "magnet/control_magnets.h"
#include "cmd/command_magnets.h"

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG == 0
#ifdef TraceDbg
#undef TraceDbg
#define TraceDbg(m, fmt, d1, d2, d3, d4, d5, d6)
#endif
#endif

static SemaphoreHandle_t seq_semaphore;
/* This pointer is used to tell if the semaphore has been initialized.  It
 * remains NULL until the seq_semaphore is initialized, then it is set to point
 * at that variable */
static SemaphoreHandle_t *pseq_semaphore = NULL;
static StaticSemaphore_t seq_semaphore_buffer;

int ctrl_throttle_step_rw(unsigned int table, unsigned int step,
                          uint32_t *val, unsigned int which, unsigned int rw)
{
    int err = 0;
    uint32_t *p = NULL;

    if(table >= THROTTLE_SEQ_COUNT || step >= SEQUENCE_MAX_STEPS_THROTTLE) {
        err = __LINE__;
    } else {
        p = (uint32_t *)&sequence_throttle_sequences[table].seq[step];
    }

    if(!err) {
        if(which == UPPER_32) {
            if(rw == SEQ_WRITE) {
                p[1] = *val;
            } else if(rw == SEQ_READ) {
                *val = p[1];
            }
        } else if(which == LOWER_32) {
            if(rw == SEQ_WRITE) {
                p[0] = *val;
            } else if(rw == SEQ_READ) {
                *val = p[0];
            }
        } else {
            err = __LINE__;
        }
    }

    return err;
}

static uint32_t seq_stat_throttle = SEQ_STAT_IDLE;
static int ctrl_throttle_seq_run(int seq_num, uint32_t setpoint)
{
    int err = 0;
    sequence_run_t seq;
    memset(&seq, 0, sizeof(seq));

    if(pseq_semaphore == NULL) {
        /* initialize the semaphore if it hasn't already happened*/
        seq_semaphore = xSemaphoreCreateBinaryStatic(&seq_semaphore_buffer);
        pseq_semaphore = &seq_semaphore;
    }

    seq.seq = sequence_throttle_sequences[seq_num].seq;
    /* This is really the max size, the seq will stop when there is a zero
     * in the sequence array */
    seq.size = SEQUENCE_MAX_STEPS_THROTTLE;
    seq.name = sequence_throttle_sequences[seq_num].name;
    seq.status = &seq_stat_throttle;
    seq.psem = pseq_semaphore;

    TraceDbg(TrcMsgSeq, "Throttle Seq%d", seq_num, 0, 0, 0, 0, 0);

    err = ctrl_sequence_throttle_run(&seq, setpoint);

    return err;
}

uint32_t ctrl_throttle_stat(void)
{
    return seq_stat_throttle;
}

uint32_t ctrl_throttle_is_throttling(void)
{
    return SEQUENCE_IS_RUNNING(seq_stat_throttle);
}

/**
 * @brief Get the appropriate throttle sequence number for the setpoint transition from current
 * to new. There are 6 throttle sequences defined and the appropriate one is defined by the
 * relationship of voltages and currents between the current and new setpoints.
 * @param setpoint New setpoint to transition to.
 * @return Throttle sequence number (1 through 6).
 */
static throttle_sequence_t ctrl_throttle_seq_num_get(uint32_t setpoint)
{
    int err = 0;
    throttle_sequence_t throttle_seq_num = THROTTLE_SEQ_1;
    thrust_data_t *ptd = NULL;

    err = thrust_table_entry_get(setpoint, &ptd);
    if(!err && ptd != NULL) {
        /* Work with the factored values - DO NOT compare floats */
        uint16_t anode_v_setpoint = ctrl_anode_volts_to_counts(ptd->anode_v);
        uint16_t magnet_i_setpoint = ctrl_magnet_amperes_to_counts(ptd->magnet_i);

        /* current as in the present value setting (not amps) */
        uint16_t anode_v_current = ctrl_anode_volts_factored_get();
        uint16_t magnet_i_current = cmd_magnet_current_factored_get();

        if(anode_v_setpoint < anode_v_current) {
            if(magnet_i_setpoint <= magnet_i_current) {
                throttle_seq_num = THROTTLE_SEQ_1;
            } else {
                /* new magnet setpoint is greater than current setting */
                throttle_seq_num = THROTTLE_SEQ_2;
            }

        } else if(anode_v_setpoint == anode_v_current) {
            if(magnet_i_setpoint <= magnet_i_current) {
                throttle_seq_num = THROTTLE_SEQ_3;
            } else {
                /* new magnet setpoint is greater than current setting */
                throttle_seq_num = THROTTLE_SEQ_4;
            }

        } else {
            /* new node voltage is greater than the current anode voltage */
            if(magnet_i_setpoint <= magnet_i_current) {
                throttle_seq_num = THROTTLE_SEQ_5;
            } else {
                /* new magnet setpoint is greater than current setting */
                throttle_seq_num = THROTTLE_SEQ_6;
            }
        }
    }
    return throttle_seq_num;
}

sequence_array_t *ctrl_throttle_seq_get(uint32_t setpoint)
{
    throttle_sequence_t throttle_seq_num = ctrl_throttle_seq_num_get(setpoint);
    return sequence_throttle_sequences[throttle_seq_num].seq;
}

/**
 * Check if Steady State can run
 * @return 0 if it can run, non-zero if not
 */
static int ctrl_throttle_can_run(void)
{
    int err = 0;
    client_update_state();// Make sure thruster state is up-to-date
    if(!THRUSTER_IN_STEADY_STATE() && Thruster_Control_State != TCS_TRANSITION_STEADY_STATE && Thruster_Control_State != TCS_CONDITIONING) {
        err = __LINE__;
    }
    return err;
}

/**
 * Throttle to the new thruster setpoint
 * @param setpoint setpoint to throttle to (indexed at ONE, i.e. 1 is the 0th entry in the table)
 * @return 0 on success, non-zero otherwise
 */
int ctrl_throttle_setpoint_set(uint32_t setpoint)
{
    int err = 0;
    throttle_sequence_t throttle_seq_num = 0;

    setpoint -= 1; /* Outside world indexes at 1 */

    if(setpoint >= thrust_table_max_valid()) {
        err = __LINE__;
    }
    if(!err) {
        err = ctrl_throttle_can_run();
    }
    if(!err) {
        throttle_seq_num = ctrl_throttle_seq_num_get(setpoint);
    }
    if(!err) {
        err = ctrl_throttle_seq_run(throttle_seq_num, setpoint);
    }
    TraceDbg(TrcMsgMcuCtl, "err:%d sp:%d sq:%d", err, setpoint + 1, throttle_seq_num, 0, 0, 0);
    return err;
}

/**
 * Throttles up/down to the next setpoint, if possible
 * 
 * NOTE: This will BLOCK until the throttle sequence finishes
 * 
 * @param increment true to increment, false to decrement
 * @return 0 on success non-zero otherwise.
 */
int ctrl_throttle_increment(bool increment)
{
    int err = 0;
    uint32_t current_setpoint = ctrl_sequence_setpoint_get(); /* Indexed at zero */
    uint32_t valid_entries = thrust_table_max_valid();

    if(increment && current_setpoint >= (valid_entries - 1)) {
        err = __LINE__;
    } else if(!increment && current_setpoint == 0) { /* decrement */
        err = __LINE__;
    } else {
        /* this is an outward facing function, so the setpoints are indexed at 1,
         * so to go up by one, tell it go up by two, it will decrement it so
         * that the actual set point indexed at zero.  When decrementing give
         * it the current setpoint and it will decrement to index the next lower
         * setpoint at zero */
        uint32_t new_setpoint = current_setpoint;
        if(increment) {
            new_setpoint = current_setpoint + 2;
        }
        err = ctrl_throttle_setpoint_set(new_setpoint);
    }
    if(!err) {
        /* Wait forever, for now. If the sequence never finishes, we have bigger
         * problems */
        err = xSemaphoreTake(*pseq_semaphore, portMAX_DELAY);
        if(err != pdTRUE) {
            TraceE3(TrcMsgMcuCtl, "throttle timeout id:0x%x to:%d", 0, 0, 0, 0, 0, 0);
            err = __LINE__;
        } else {
            err = 0;
        }
    }
    return err;
}
