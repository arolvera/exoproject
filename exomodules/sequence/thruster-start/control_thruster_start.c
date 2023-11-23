/**
 * @file    control_thruster_start.c
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

#include "thruster-start/control_thruster_start.h"
#include "client-control/client_control.h"
#include "keeper/control_keeper.h"
#include "setpoint/control_setpoint.h"
#include "trace/trace.h"// Trace messages

/************** Sequence Setters/Getters ************************/
int ctrl_ts_step_rw(unsigned int table, unsigned int step,
        uint32_t* val, unsigned int which, unsigned int rw)
{
    int err = 0;
    uint32_t* p = NULL;

    if(table == SEQ_MODE_READY && step < SIZEOF_ARRAY(sequence_ready_mode)){
        p = (uint32_t*)&sequence_ready_mode[step];
    } else if(table == SEQ_MODE_STEADY_STATE && step < SIZEOF_ARRAY(sequence_steady_state)){
        p = (uint32_t*)&sequence_steady_state[step];
    } else {
        err = __LINE__;
    }

    if(!err){
        if(which == UPPER_32){
            if(rw == SEQ_WRITE){
                p[1] = *val;
            } else if(rw == SEQ_READ){
                *val = p[1];
            }
        } else if(which == LOWER_32){
             if(rw == SEQ_WRITE){
                p[0] = *val;
            } else if(rw == SEQ_READ){
                *val = p[0];
            }
        } else {
            err = __LINE__;
        }
    }

    return err;

}

int ctrl_ts_steady_state_set(unsigned int index, uint64_t value)
{
    int err = 0;
    if(index >= SEQUENCE_MAX_STEPS_ANODE) {
        err = -1;
    } else {
        sequence_steady_state[index] = value;
    }
    return err;
}

uint32_t ctrl_ts_steady_state_get(unsigned int index)
{
    uint32_t ret = -1;
    if(index < SEQUENCE_MAX_STEPS_ANODE) {
        ret = sequence_ready_mode[index];
    }
    return ret;
}

int ctrl_ts_ready_mode_set(unsigned int index, uint64_t value)
{
    int err = 0;
    if(index >= SEQUENCE_MAX_STEPS_KEEPER) {
        err = -1;
    } else {
        sequence_steady_state[index] = value;
    }
    return err;
}

uint32_t ctrl_ts_ready_mode_get(unsigned int index)
{
    uint32_t ret = -1;
    if(index < SEQUENCE_MAX_STEPS_KEEPER) {
        ret = sequence_ready_mode[index];
    }
    return ret;
}

typedef int (*SEQ_STEP_SET) (unsigned int idx, uint64_t value);

/**
 * Must be in sequence_t enum order
 * ToDo: This is not used anywhere. Complete the implementation or get rid of it.
 */
SEQ_STEP_SET seq_step_set_funcs[] = {
    ctrl_ts_steady_state_set,
    ctrl_ts_ready_mode_set
    /* @todo add step set functions for below functions */
};
#define SEQ_STEP_SET_ARRAY_SIZE (sizeof(seq_step_set_funcs)/sizeof(SEQ_STEP_SET))

/************** END Sequence Setters/Getters **********************************/

/************** Sequence Safety   *********************************************/

/**
 * Check if Ready Mode can run
 * @return 0 if it can run, non-zero if not
 */
static int ctrl_ts_ready_mode_can_run(void)
{
    int err = 0;
    client_update_state(); // Make sure thruster state is up-to-date
    if(Thruster_Control_State != TCS_STANDBY) {
        err = __LINE__;
    }
    return err;
}
/**
 * Check if Steady State can run
 * @return 0 if it can run, non-zero if not
 */
static int ctrl_ts_steady_state_can_run(void)
{
    int err = 0;
    client_update_state(); // Make sure thruster state is up-to-date
    if(Thruster_Control_State != TCS_READY_MODE &&
       Thruster_Control_State != TCS_TRANSITION_READY_MODE  ) {
        err = __LINE__;
    }
    return err;
}


/************** END Sequence Safety   *****************************************/

/************** Sequence Runners   ********************************************/

static uint32_t seq_stat_ready_mode = SEQ_STAT_IDLE;
int ctrl_ts_ready_mode_run(void)
{
    int err = 0;
    sequence_run_t seq;
    
    err = ctrl_ts_ready_mode_can_run();    
    if(!err) {
        memset(&seq, 0, sizeof(seq));
        seq.seq = sequence_ready_mode;
        seq.size = sizeof(sequence_ready_mode)/sizeof(sequence_array_t);
        seq.name = NAME_READY_MODE;
        seq.status = &seq_stat_ready_mode;

        err = ctrl_sequence_run(&seq);
    }
    return err;
}
uint32_t ctrl_ts_ready_mode_stat(void)
{
    return seq_stat_ready_mode;
}

static uint32_t seq_stat_steady_state = SEQ_STAT_IDLE;
int ctrl_ts_steady_state_run(uint32_t setpoint)
{
    int err = 0;
    sequence_run_t seq;

    if(setpoint > THRUST_POINTS_MAX) {
        err = __LINE__;
    }
    if(!err) {
        err = ctrl_ts_steady_state_can_run();
    }
    if(!err) {
        memset(&seq, 0, sizeof(seq));

        seq.seq = sequence_steady_state;
        seq.size = sizeof(sequence_steady_state)/sizeof(sequence_array_t);
        seq.name = NAME_STEADY_STATE;
        seq.status = &seq_stat_steady_state;

        /* Steady state errors need to be cleared because they are monitored.
         * Things that are monitored only get 'set' once, if they do not get
         * cleared here, they will not get set again properly.  For example,
         * if you get a 'power too high' and leave it set, it will not get set
         * again the next time you try steady state.  Then power will run away,
         * and the EE's tell me that is bad.
         */
        client_steady_state_err_clr();

        /* Anode starts at throttle point */
        err = ctrl_sequence_throttle_run(&seq, setpoint);
    }
    return err;
}

uint32_t ctrl_ts_steady_state_stat(void)
{
    return seq_stat_steady_state;
}

uint32_t ctrl_ts_in_transition(void)
{
    uint32_t tcs = TCS_CO_INVALID;
    if( SEQUENCE_IS_RUNNING(seq_stat_ready_mode) ) {
        tcs = TCS_TRANSITION_READY_MODE;

    } else if( SEQUENCE_IS_RUNNING(seq_stat_steady_state) ) {
        tcs = TCS_TRANSITION_STEADY_STATE;
    }
    return tcs;
}
