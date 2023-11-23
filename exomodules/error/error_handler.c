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

#include <limits.h>
#include "definitions.h"
#include "trace/trace.h"
#include "error_handler.h"

/*
 * Adjust to account for more modules that need error handling
 */
#define MAX_REGISTERED_MODULES 10
#define MAX_LOG_ENTRIES        100

typedef enum {
  EH_INCREMENT,
  EH_DECREMENT
} inc_dec_t;

static OSAL_MUTEX_HANDLE_TYPE err_cnt_mtx;

/* Bitmask for fault statuses */
uint32_t fault_status[5];

static error_detail_t error_detail_register[MAX_REGISTERED_MODULES];

/* Helper struct for fault bit setting */
typedef struct {
  unsigned int e_code;
  unsigned int index;
  unsigned int bit_shift;
} eh_fault_pos_t;

/**
 * Return full error detail for module/submodule pair
 * 
 * 
 * @param module number 
 * @param submodule number
 * @return pointer to full error detail if found, NULL if not found
 */
error_detail_t *eh_get(uint8_t module, uint8_t submodule)
{
    uint32_t iterator = 0;
    error_detail_t *p = NULL;
    for(; iterator < SIZEOF_ARRAY(error_detail_register) && !p; iterator++) {
        if(error_detail_register[iterator].module == module) {
            p = &error_detail_register[iterator];
        }
    }

    return p;
}

/**
 * Find bit position of fault register in terms of index and bit offset
 * 
 * 
 * @param bit position helper struct
 * @return success or error
 */
static int eh_fault_status_pos_get(eh_fault_pos_t *fault_pos)
{
    int err = 0;
    uint32_t index = 0;
    uint32_t bit_shift = 0;

    /*
     * Find bits per status word
     */
    uint32_t bits = sizeof(fault_status[0]) * CHAR_BIT;

    /*
     * find bit offset in fault_Status[index]
     * Subtract 1 because our error handling scheme starts assigning bit 0,1,2,3.....
     * at error code 1,2,3... so the bit mask will have an off-by-one nuance relative
     * to the actual error code
     */
    if(fault_pos->e_code > 0) {
        bit_shift = ((fault_pos->e_code - 1) % bits);
        index = (fault_pos->e_code - 1) / bits;
    }else {
        err = __LINE__;
    }

    if(!err && index > SIZEOF_ARRAY(fault_status)) {
        err = __LINE__;
    }

    if(!err && fault_pos->e_code > ERROR_CODE_EOL) {
        err = __LINE__;
    }

    if(!err && fault_pos->e_code > (sizeof(fault_status) * CHAR_BIT)) {
        err = __LINE__;
    }

    if(!err) {
        fault_pos->bit_shift = bit_shift;
        fault_pos->index = index;
    }
    return err;
}

/**
 * Set bit in fault status mask
 * 
 * 
 * @param bit to set
 * @return error status
 */
int eh_fault_status_set(unsigned int e_type, unsigned int e_code)
{
    int err;

    eh_fault_pos_t fault_pos = {.e_code = e_code};

    err = eh_fault_status_pos_get(&fault_pos);

    /*
     * If no error and the error bit isn't already set
     */
    if(!err && !(fault_status[fault_pos.index] & (1 << fault_pos.bit_shift))) {
        fault_status[fault_pos.index] |= (1 << fault_pos.bit_shift);
    }

    return err;
}

/**
 * Clear bit in fault status mask
 * 
 * 
 * @param bit to set
 * @return error status
 */
int eh_fault_status_clear(unsigned int e_type, unsigned int e_code)
{
    int err;

    eh_fault_pos_t fault_pos = {.e_code = e_code};

    err = eh_fault_status_pos_get(&fault_pos);

    /*
     * If no error and the error bit is already set
     */
    if(!err && (fault_status[fault_pos.index] & (1 << fault_pos.bit_shift))) {
        fault_status[fault_pos.index] &= ~(1 << fault_pos.bit_shift);
    }

    return err;
}

/**
 * Check if bit is set in fault status mask
 * 
 * 
 * @param bit to set
 * @return error status
 */
int eh_fault_status_get(unsigned int e_code)
{
    eh_fault_pos_t fault_pos = {.e_code = e_code};

    eh_fault_status_pos_get(&fault_pos);

    return (int)fault_status[fault_pos.index] & (1 << fault_pos.bit_shift);
}

/**
 * Get info about fault status
 * 
 * 
 * @param pointer to set size of fault bitmask
 * @return error pointer to bitmask
 */
int eh_fault_stat(fault_status_reg_stat_t *fr_stat)
{
    int err = 0;
    if(fr_stat == NULL) {
        err = __LINE__;
    }else {
        fr_stat->size = sizeof(fault_status);
        fr_stat->reg_start = (uint8_t *)fault_status;
    }
    return err;
}

/**
 * Subscribe client to error handler service
 * 
 * 
 * @param module to subscribe
 * @param submodule to subscribe
 * @param modules strategy
 * @param length of modules log
 * @param pointer to modules log
 * @return error status
 */
int eh_create(uint8_t module, uint8_t submodule, ERROR_DETAIL_STRATEGY err_detail_strategy, log_length_t log_size,
    void *e_log)
{
    static unsigned int slot = 0;
//    static unsigned int mega_log_slice = 0;               assigned but never used
    int err = 0;
    /* make sure there's an open spot to register the module in */
    if(slot < SIZEOF_ARRAY(error_detail_register) &&
        log_size != 0 &&
        ((log_size & (log_size - 1)) == 0)) {
        /* Create the module error handler in the next open slot*/
        error_detail_register[slot].error_count = 0;
        error_detail_register[slot].log_head = 0;
        error_detail_register[slot].error_detail_log_len = log_size;
        error_detail_register[slot].module = module;
        error_detail_register[slot].submodule = submodule;
        error_detail_register[slot].error_detail_strategy = err_detail_strategy;
        error_detail_register[slot].error_log = e_log;
        slot++;
//        mega_log_slice += log_size;
    }else {
        err = __LINE__;
    }

    return err;
}

/**
 * Get modules strategy
 * 
 * 
 * @param module to subscribe
 * @param submodule to subscribe
 * @param pointer to fill in size of module's log
 * @param pointer to fill in log's head
 * @return function pointer to module's strategy
 */
ERROR_DETAIL_STRATEGY eh_module_strategy_get(int module, int submodule, int *log_size, int *log_head)
{
    int found = 0;
    ERROR_DETAIL_STRATEGY fp = NULL;

    for(uint32_t i = 0; i < SIZEOF_ARRAY(error_detail_register) && !found; i++) {
        if(module == error_detail_register[i].module && submodule == error_detail_register[i].submodule) {
            *log_head = error_detail_register[i].log_head;
            *log_size = (int)error_detail_register[i].error_detail_log_len;
            fp = error_detail_register[i].error_detail_strategy;
            found = 1;
        }
    }

    return fp;
}

void eh_init(void)
{
    static StaticSemaphore_t err_mtx_buff;
    OSAL_MUTEX_Create(&err_cnt_mtx, &err_mtx_buff, "error");
}

/* This is a function that gets called from a macro.  Having a trace message in a 
 * macro duplicates the code everywhere the macro is used.  That makes the
 * binary larger and our memory footprint larger
 */
void __attribute__ ((noinline)) eh_set_trc_msg(int mod, int sub, int line, int e_cnt, int e_type__, int e_code__)
{
    TraceE2(TrcMsgErr2, "ERROR_SET mod: 0x%x:0x%x, count:%d, line: %d, type: 0x%x, code: 0x%x",
            mod, sub, e_cnt, line, e_type__, e_code__);
}

/* This is a function that gets called from a macro.  Having a trace message in a 
 * macro duplicates the code everywhere the macro is used.  That makes the
 * binary larger and our memory footprint larger
 */
void __attribute__ ((noinline)) eh_clr_trc_msg(int mod, int sub, int e_cnt, int line, int e_type__, int e_code__)
{
    TraceE2(TrcMsgErr2, "ERROR_CLR mod0x%x:0x%x, count:%d, line: %d, type: 0x%x, code: 0x%x",
            mod, sub, e_cnt, line, e_type__, e_code__);
}

