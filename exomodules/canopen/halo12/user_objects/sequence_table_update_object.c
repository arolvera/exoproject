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
/**
 * anode_startup_object.c
 *
 * @Company Exoterra
 * @File anode_startup_object.c
 * @Summary  see header  description
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "app_dict.h"// For adding Oject Dictionary Entires
#include "client_control/control_bit.h"
#include "client_control/control_condition.h"
#include "client_control/control_sequence.h"
#include "client_control/control_thruster_start.h"
#include "client_control/control_condition.h"
#include "client_control/control_throttle.h"
#include "client_control/control_bit.h"
#include "user_object_od_indexes.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */


/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */


static CO_ERR   SqncCtrlWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   SqncCtrlRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

/** 
 * AnodeStart Control User Object
 */
CO_OBJ_TYPE SqncCtrlType = {
    0,                 /* type function to get object size      */
    0,                 /* type function to control type object  */
    SqncCtrlRead,      /* type function to read object content  */
    SqncCtrlWrite,     /* type function to write object content */
};
#define CO_TSQNC_CTRL  ((CO_OBJ_TYPE*)&SqncCtrlType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    SQNC_CTRL_SUBIDX_COUNT = 0,
    SQNC_CTRL_STEP_COUNT,           /* Number of valid steps in the table */
    SQNC_CTRL_EXECUTE,              /* Execute the sequence               */
    SQNC_CTRL_CONDITION,            /* Execute the condition sequence     */
    SQNC_CTRL_TABLE_SELECT,
    SQNC_CTRL_STEP_SELECT,      
    SQNC_CTRL_STEP_ENTRY_WRITE_UPPER_32,
    SQNC_CTRL_STEP_ENTRY_WRITE_LOWER_32,
    SQNC_CTRL_SEQUENCE_BASE,        /* The first index of the sequence    */
    SQNC_CTRL_SUBIDX_EOL = SQNC_CTRL_SEQUENCE_BASE + SEQUENCE_MAX_STEPS_ANODE,
} OD_SQNC_CTRL;


typedef int (*table_func)(unsigned int table, unsigned int step, 
        uint32_t* val, unsigned int which, unsigned int rw);

static table_func table_rw     = NULL;
static sequence_t table_select = 0;
static uint32_t   row_select   = 0;

void SqncCtrlOD(OD_DYN *self)
{
    /* AnodeStart Control */
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_SUBIDX_COUNT,
                CO_UNSIGNED16 |CO_OBJ____R_), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_STEP_COUNT,
                CO_UNSIGNED16 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_EXECUTE,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_CONDITION,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_TABLE_SELECT,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_STEP_SELECT,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_STEP_ENTRY_WRITE_UPPER_32,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    ODAdd(self, CO_KEY(OD_INDEX_SQNC_CTRL, SQNC_CTRL_STEP_ENTRY_WRITE_LOWER_32,
                CO_UNSIGNED32 |CO_OBJ____RW), CO_TSQNC_CTRL, (uintptr_t)&SqncCtrlType);
    
}

/**
 * Write to the anode variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR SqncCtrlWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint32_t step_upper_32 = 0;
    uint32_t step_lower_32 = 0;
    
     if(subidx == SQNC_CTRL_TABLE_SELECT) {
        table_select  = *(uint32_t*)buf;
        if(table_select < SEQ_MODE_EOL){
            /* Already at table index 0, no need to adjusted table select index*/
            table_rw = ctrl_ts_step_rw;
        } else if(table_select >= SEQ_THROTTLE_1 && table_select < SEQ_EOL){
            /* Adjust table selector so that table index is 0 instead of value in enum */
            /* NOTE: will need to change if SEQ_THROTTLE_1 changes position in enum */
            table_select = table_select - SEQ_THROTTLE_1;
            table_rw = ctrl_throttle_step_rw;
        } else if(table_select >= SEQ_COND_MAGS && table_select < SEQ_COND_EOL){
            /* Adjust table selector so that table index is 0 instead of value in enum */
            /* NOTE: will need to change if SEQ_COND_MAGS changes position in enum */
            table_select = table_select - SEQ_COND_MAGS;
            table_rw = ctrl_condition_step_rw;
        } else if(table_select >= SEQ_BIT_USER_MOD && table_select < SEQ_BIT_EOL){
            /* Adjust table selector so that table index is 0 instead of value in enum */
            /* NOTE: will need to change if SEQ_BIT_USER_MOD changes position in enum */
            table_select = table_select - SEQ_BIT_USER_MOD;
            table_rw = ctrl_bit_step_rw;
        }
    } else if(subidx == SQNC_CTRL_STEP_SELECT) {
        row_select    = *(uint32_t*)buf;
    } else if(table_rw != NULL && subidx == SQNC_CTRL_STEP_ENTRY_WRITE_UPPER_32) {
        step_upper_32 = *(uint32_t*)buf;
        write_err = table_rw(table_select, row_select, &step_upper_32, UPPER_32, SEQ_WRITE);
    } else if(table_rw != NULL && subidx == SQNC_CTRL_STEP_ENTRY_WRITE_LOWER_32) {
        step_lower_32 = *(uint32_t*)buf;
        write_err = table_rw(table_select, row_select, &step_lower_32, LOWER_32, SEQ_WRITE);
    } else {
        write_err = __LINE__; 
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * Write to the anode variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR SqncCtrlRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{

    
    int read_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint32_t step_upper_32 = 0;
    uint32_t step_lower_32 = 0;
    
    if(subidx == SQNC_CTRL_TABLE_SELECT) {
         *(uint32_t*)buf = table_select;
    } else if(subidx == SQNC_CTRL_STEP_SELECT) {
        *(uint32_t*)buf = row_select;
    } else if(table_rw != NULL && subidx == SQNC_CTRL_STEP_ENTRY_WRITE_UPPER_32) {
        read_err = table_rw(table_select, row_select, &step_upper_32, UPPER_32, SEQ_READ);
        *(uint32_t*)buf = step_upper_32;
    } else if(table_rw != NULL && subidx == SQNC_CTRL_STEP_ENTRY_WRITE_LOWER_32) {
        read_err = table_rw(table_select, row_select, &step_lower_32, LOWER_32, SEQ_READ);
        *(uint32_t*)buf = step_lower_32;
    } else {
        read_err = __LINE__; 
    }
    
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/* *****************************************************************************
 End of File
 */
