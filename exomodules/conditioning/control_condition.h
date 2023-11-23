/**
 * @file    control_condition.h
 *
 * @brief   ???
 *
 * Why do we need this?
 *   halo 12 will not require conditioning
 *   Can I just remove everything from this folder as well as canopen objects?
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

#ifndef CONTROL_CONDITION_H
#define	CONTROL_CONDITION_H

#ifdef	__cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <stdbool.h>
//#include "seq_cond.h"
#include "seq_cond_definition.h"
    
ctrl_cond_seq_ucv_t *ctrl_condition_seq_ucv_get(uint32_t el);
int ctrl_condition_num_sequence_get(void);

int ctrl_condition_status_clear(void);

int ctrl_condition_start(uint32_t start_step);
int ctrl_condition_abort(void);
uint32_t ctrl_condition_stat(void);
int ctrl_condition_nvm_reset(void);
int ctrl_condition_step_rw(unsigned int table, unsigned int step, 
uint32_t* val, unsigned int which, unsigned int rw);
int ctrl_condition_stats_get(condition_steps_stat_t *);

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_CONDITION_H */

