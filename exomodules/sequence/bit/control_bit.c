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
#include "client-control/client_control.h"
#include "control_bit.h"

static uint32_t seq_stat_bit = SEQ_STAT_IDLE;
static uint32_t seq_bit_num = 0; /* Which BIT is running */

int ctrl_bit_step_rw(unsigned int table, unsigned int step,
        uint32_t* val, unsigned int which, unsigned int rw)
{
    int err = 0;
    uint32_t* p = NULL;
    
    if(table >= bit_seq_size || step > bit_seqs[table].size ) {
        err = __LINE__;
    } else {
        p = (uint32_t*)&bit_seqs[table].seq[step];
        
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

/************** Sequence Safety   *********************************************/

/**
 * Check if Ready Mode can run
 * @return 0 if it can run, non-zero if not
 */
static int ctrl_bit_can_run(void)
{
    int err = 0;
    client_update_state(); // Make sure thruster state is up-to-date
    if(Thruster_Control_State != TCS_STANDBY) {
        err = __LINE__;
    }
    return err;
}


/************** END Sequence Safety   *****************************************/

/************** Sequence Runners   ********************************************/

/**
 * Check if BIT is running
 * @return 0 if not running, BIT number if it is running
 */
uint32_t ctrl_bit_isrunning(void)
{
    return SEQUENCE_STATUS_GET(seq_stat_bit) == SEQ_STAT_QUEUED ||
           SEQUENCE_STATUS_GET(seq_stat_bit) == SEQ_STAT_RUNNING;
}

int ctrl_bit_run(uint32_t bit_test_num)
{
    int err = 0;
    bit_test_num -= 1;
    uint32_t nbits  = bit_seq_size;
    sequence_run_t seq;
    
    if(bit_test_num >= nbits) {
        err = __LINE__;
    }
    if(!err) {
        err = ctrl_bit_can_run();
    }
    if(!err) {
        memset(&seq, 0, sizeof(seq));
        
        seq.seq = bit_seqs[bit_test_num].seq;
        seq.size =  bit_seqs[bit_test_num].size;
        seq.name = bit_seqs[bit_test_num].name;
        seq.status = &seq_stat_bit;
        
        /* Anode starts at throttle point */
        err = ctrl_sequence_run(&seq);
    }
    if(!err) {
        seq_bit_num = bit_test_num;
    }
    return err;
}

uint32_t ctrl_bit_stat(void)
{
    return seq_stat_bit;
}

/************** End Sequence Runners   **********************************/

