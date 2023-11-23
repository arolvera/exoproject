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

#ifndef HARDWARECONTROL_HALO12_VA41630_EXOMODULES_CONDITIONING_SEQ_COND_DEFINITION_H_
#define HARDWARECONTROL_HALO12_VA41630_EXOMODULES_CONDITIONING_SEQ_COND_DEFINITION_H_
#include "sequence/control_sequence.h"
//#include "thruster_control.h"

/* To speed up the process during test and integration */
#if __DEBUG
#define DELAY_FACTOR 10
#else
#define DELAY_FACTOR 1000
#endif

typedef struct _condition_stat {
    uint32_t seq_stat_cond;         /* Status of the steps sequence           */
    uint32_t elapsed_ms;            /* Milliseconds that it has been monitored*/
    uint32_t monitor_err;           /* Error during the monitoring phase      */
} condition_stat_t;

typedef enum {
    CONDITION_RUN_STAT_IDLE,
    CONDITION_RUN_STAT_RUNNING,
    CONDITION_RUN_STAT_FAILED,
    CONDITION_RUN_STAT_ABORTED,
    CONDITION_RUN_STAT_PASSED,
    CONDITION_RUN_STAT_RETRY_DELAY,
} condition_step_status_t;

typedef struct _condition_steps_stat {
    uint32_t count;                 /* How many steps                         */
    uint32_t *step;                 /* What step is it on?                    */
    condition_stat_t *step_stats;   /* Pointer to base of the stats array     */
} condition_steps_stat_t;

typedef struct sequence_limits {
    uint32_t max_limit;             /* Absolute max - turn it off now       */
    uint32_t adjust_limit_upper;    /* Upper limit before adjusting down    */
    uint32_t adjust_limit_lower;    /* Lower limit before adjusting up      */
    uint32_t current_limit;         /* Component max ampere limit           */
    uint32_t voltage_limit;         /* Component max voltage limit          */
    uint32_t power_limit;           /* Component max power limit            */
} sequence_limits_t;

//Data to be saved in user config var nvm area
#pragma pack(push,1)
typedef struct {
    sequence_limits_t limits;
    uint32_t monitor_ms;  /* Monitor time is fixed, elapse time is tracked elsewhere */
}ctrl_cond_seq_ucv_t;
#pragma pack(pop)

typedef enum {
    CONDITION_MAGNET,
    CONDITION_KEEPER,
    CONDITION_ANODE,
    CONDITION_ANODE_THROTTLE,
} condition_sequence_t;

typedef struct sequence_condition {
    sequence_array_t *seq;
    char *name;
    unsigned int size;
    ctrl_cond_seq_ucv_t seq_ucv;
    condition_sequence_t type;
    int required_state;
    condition_stat_t *stat;
} sequence_condition_t;

EXT_DECL sequence_condition_t sequence_condition_sequences[];
#endif//HARDWARECONTROL_HALO12_VA41630_EXOMODULES_CONDITIONING_SEQ_COND_DEFINITION_H_
