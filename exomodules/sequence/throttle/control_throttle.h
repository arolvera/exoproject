/**
 * @file    control_sequence.h
 *
 * @brief   Interface for the Halo and Halo 12 throttle control functions.
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

#ifndef CONTROL_THROTTLE_H
#define	CONTROL_THROTTLE_H

#include "ext_decl_define.h"
#include "definitions.h"
#include "sequence/control_sequence.h"
#include "sequence_throttle.h"

#ifdef	__cplusplus
extern "C" {
#endif

//Defined in control_throttle_<project>_<gas>_<coil>_<customer>.c
EXT_DECL sequence_throttle_t *sequence_throttle_sequences;

int ctrl_throttle_step_rw(unsigned int table, unsigned int step, 
        uint32_t* val, unsigned int which, unsigned int rw);
int ctrl_throttle_setpoint_set(uint32_t setpoint);
int ctrl_throttle_increment(bool increment);

uint32_t ctrl_throttle_setpoint_get(void);
uint32_t ctrl_throttle_stat(void);
uint32_t ctrl_throttle_is_throttling(void);
sequence_array_t *ctrl_throttle_seq_get(uint32_t setpoint);

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_THROTTLE_H */

