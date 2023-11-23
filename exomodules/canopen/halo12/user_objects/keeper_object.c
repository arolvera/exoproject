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
 * keeper_object.c
 *
 * @Company Exoterra
 * @File keeper_object.c
 * @Summary  User object for keeper control.
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */
#include <string.h>                 // memcpy
#include "keeper_object.h"                           // Header for this module
#include "keeper/control_keeper.h"           // Keeper control module
#include "thruster_control.h"
#include "hsi_memory.h"
#include "user_object_od_indexes.h"

static CO_ERR   KeeperWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   KeeperRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t KeeperSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);

//static uint16_t* health_history_buf;
/** 
 * Keeper Control User Object
 */
CO_OBJ_TYPE KeeperType = {
    KeeperSize,    /* type function to get object size      */
    0,             /* type function to control type object  */
    KeeperRead,    /* type function to read object content  */
    KeeperWrite,   /* type function to write object content */
};
#define CO_TKEEPER  ((CO_OBJ_TYPE*)&KeeperType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    KEEPER_SUBIDX_COUNT,
    KEEPER_SUBIDX_VOLTS,
    KEEPER_SUBIDX_VOLTS_FACTORED,
    KEEPER_SUBIDX_CUR,
    KEEPER_SUBIDX_CUR_FACTORED,
    KEEPER_SUBIDX_POWER_SUPPLY,
    KEEPER_SUBIDX_STATE,         /* Current Anode State */
    KEEPER_SUBIDX_STATE_STAT,    /* State changes come with a status as to why they changed state */
    KEEPER_SUBIDX_EOL,
} OD_KEEPER;

typedef enum {
    KEEPER_DIAG_SUBIDX_COUNT,
    KEEPER_DIAG_SUBIDX_VOUT,   //SEPIC
    KEEPER_DIAG_SUBIDX_VIN,
    KEEPER_DIAG_SUBIDX_IOUT,
    KEEPER_DIAG_SUBIDX_CURR_STATE,
    KEEPER_DIAG_SUBIDX_ERR_CODE,
    KEEPER_DIAG_SUBIDX_PWM_OUT,
    KEEPER_DIAG_SUBIDX_TEMP,
    KEEPER_DIAG_SUBIDX_EOL
} OD_KEEPER_DIAG;

/**
 * Write to the keeper variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR KeeperWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case KEEPER_SUBIDX_VOLTS :
            write_err = ctrl_keeper_volts_set(*((float *)buf));
            break;
        case KEEPER_SUBIDX_CUR   : 
            write_err = ctrl_keeper_cur_set(*((float *)buf));
            break;
        case KEEPER_SUBIDX_POWER_SUPPLY:
            write_err = ctrl_keeper_ps_state_set(*((uint8_t *)buf));
            break;
        case KEEPER_SUBIDX_COUNT : /* Read-only - fall through to error */
        case KEEPER_SUBIDX_VOLTS_FACTORED:
        case KEEPER_SUBIDX_CUR_FACTORED:
        case KEEPER_SUBIDX_STATE:
        case KEEPER_SUBIDX_STATE_STAT:
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * Write to the keeper variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR KeeperRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    uint16_t idx    = CO_GET_IDX(obj->Key);
    
   if(idx == OD_INDEX_KEEPER){
        switch(subidx) {
            case KEEPER_SUBIDX_VOLTS :
                *((float *)buf) = ctrl_keeper_volts_get();
                break;
            case KEEPER_SUBIDX_VOLTS_FACTORED :
                *((uint16_t *)buf) = ctrl_keeper_volts_factored_get();
                break;
            case KEEPER_SUBIDX_CUR :
                *((float *)buf) = ctrl_keeper_cur_get();
                break;
            case KEEPER_SUBIDX_CUR_FACTORED :
                *((uint16_t *)buf) = ctrl_keeper_cur_factored_get();
                break;
            case KEEPER_SUBIDX_POWER_SUPPLY :
                *((uint8_t *)buf) = ctrl_keeper_ps_state_get();
                break;
            case KEEPER_SUBIDX_STATE :
                *((uint8_t *)buf) = ctrl_keeper_state_get();
                break;
            case KEEPER_SUBIDX_STATE_STAT :
                *((uint8_t *)buf) = ctrl_keeper_state_stat_get();
                break;
            case KEEPER_SUBIDX_COUNT :
                *((uint32_t *)buf) = KEEPER_SUBIDX_EOL-1;
                break;
            default:
                read_err = __LINE__; // Not implemented
        }
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}
/**
 * Size callback for user object.  The CAN Open stack needs to know the size
 * of each object
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return size of the field in bytes
 */
static uint32_t KeeperSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    switch(subidx) {
        case KEEPER_SUBIDX_VOLTS :
            size = sizeof(((control_keeper_t *)0)->voltage);
            break;
        case KEEPER_SUBIDX_VOLTS_FACTORED :
            size = sizeof(((control_keeper_t *)0)->voltage_factored);
            break;
        case KEEPER_SUBIDX_CUR :
            size = sizeof(((control_keeper_t *)0)->current);
            break;
        case KEEPER_SUBIDX_CUR_FACTORED :
            size = sizeof(((control_keeper_t *)0)->current_factored);
            break;
        case KEEPER_SUBIDX_POWER_SUPPLY :
            size = sizeof(((control_keeper_t *)0)->ps_state);
            break;
        case KEEPER_SUBIDX_COUNT :
            size = sizeof(uint8_t);
            break;
        default:
            size = 0; // Not implemented
    }
    return size;
}

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */
void KeeperOD(OD_DYN *self)
{
    
    /* Keeper Control */
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_COUNT,
                CO_UNSIGNED16|CO_OBJ____R_), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_VOLTS,
                CO_FLOAT     |CO_OBJ____RW), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_VOLTS_FACTORED,
                CO_UNSIGNED16|CO_OBJ____R_), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_CUR,
                CO_FLOAT     |CO_OBJ____RW), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_CUR_FACTORED,
                CO_UNSIGNED16|CO_OBJ____R_), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_POWER_SUPPLY,
                CO_UNSIGNED8 |CO_OBJ____RW), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_STATE,
                CO_UNSIGNED8 |CO_OBJ____R_), CO_TKEEPER, (uintptr_t)&KeeperType);
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER, KEEPER_SUBIDX_STATE_STAT,
                CO_UNSIGNED8 |CO_OBJ____R_), CO_TKEEPER, (uintptr_t)&KeeperType);
    

    // ODs for keeper DIAG
    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_COUNT, CO_UNSIGNED8  |CO_OBJ_D__R_), 0, KEEPER_DIAG_SUBIDX_EOL - 1);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_VOUT, CO_UNSIGNED32 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.vout);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_VIN, CO_UNSIGNED16 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.vin);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_IOUT, CO_UNSIGNED16 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.iout);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_CURR_STATE, CO_UNSIGNED16 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.current_state);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_ERR_CODE, CO_UNSIGNED16 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.error_code);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_PWM_OUT, CO_UNSIGNED16 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.pwm_output);

    ODAdd(self, CO_KEY(OD_INDEX_KEEPER_DIAG, KEEPER_DIAG_SUBIDX_TEMP, CO_UNSIGNED16 | CO_OBJ____R_),
            0, (uintptr_t)&hsi_mem.keeper_telem.temperature);

//
//    int entries = ctrl_keeper_health_history_buf_get(&health_history_buf);
//
//    for(int i = 0; i < entries /  sizeof(uint16_t) && health_history_buf != NULL; i++) {
//        ODAdd(self, CO_KEY(OD_INDEX_KEEPER_ERR_HISTORY, i, CO_UNSIGNED16 | CO_OBJ____R_),
//                0, (uintptr_t)(&health_history_buf[i]));
//    }
}
/* *****************************************************************************
 End of File
 */
