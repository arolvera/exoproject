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
 * File:   control_thruster_start.h
 * Author: jmeyers
 *
 * Created on September 23, 2021, 6:24 AM
 */

#ifndef CONTROL_BIT_TEST_H
#define	CONTROL_BIT_TEST_H
#include <stdint.h>
#include "sequence/control_sequence.h"

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct sequence_bit_t_{
    sequence_array_t * seq;
    char *name;
    uint32_t size;
}sequence_bit_t;

//Define in sequence_bit_<project>.c
EXT_DECL sequence_bit_t bit_seqs[];
EXT_DECL size_t bit_seq_size;

uint32_t ctrl_bit_stat(void);
uint32_t ctrl_bit_get(unsigned int index);
int ctrl_bit_step_rw(unsigned int table, unsigned int step, uint32_t* val,
        unsigned int which, unsigned int rw);
uint32_t ctrl_bit_isrunning(void);
int ctrl_bit_run(uint32_t bit_test_num);

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROL_THRUSTER_START_H */

