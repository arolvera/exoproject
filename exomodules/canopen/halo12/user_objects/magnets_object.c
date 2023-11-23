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
 * magnet_object.c
 *
 * @Company Exoterra
 * @File update_object.c
 * @Summary  User object for magnet control.
 * @Description User object function definitions for receiving a software
 * update file via the CAN Open interface.  The functions here are called when
 * the user accesess the corresponding Object Dictionary item. 
 * 
 * For details, see the canopen-stack documentation here:
 * https://canopen-stack.org/docs/usecase/dictionary
 * 
 */

#include <string.h>                 // memcpy

#include "magnets_object.h"
#include "cmd/command_magnets.h"           // Magnet command module
#include "magnet/control_magnets.h"           // Magnet control module
#include "hsi_memory.h"

static CO_ERR   MagnetWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   MagnetRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t MagnetSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);

/** 
 * 0x2300 - Magnet Control User Object
 */
CO_OBJ_TYPE MagnetType = {
    MagnetSize,   /* type function to get object size      */
    0,            /* type function to control type object  */
    MagnetRead,   /* type function to read object content  */
    MagnetWrite,  /* type function to write object content */
};
#define CO_TMAGNET  ((CO_OBJ_TYPE*)&MagnetType)


/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    MAGNET_SUBIDX_COUNT                     = 0,
    MAGNET_SUBIDX_CURRENT                   = 1,
    MAGNET_SUBIDX_CURRENT_FACTORED          = 2,
    MAGNET_SUBIDX_STATE                     = 3,
    MAGNET_SUBIDX_EOL                       = 4,
} OD_MAGNET;

/**
 * Write to the magnet variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR MagnetWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case MAGNET_SUBIDX_CURRENT :
            write_err = cmd_magnet_current_set(*((float *)buf));
            break;
        case MAGNET_SUBIDX_STATE :
            write_err = cmd_magnet_state_set(*((uint8_t *)buf));
            break;
        case MAGNET_SUBIDX_COUNT : /* Read-only - fall through to error */
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * Write to the magnet variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR MagnetRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    uint16_t idx    = CO_GET_IDX(obj->Key);
    
    if(idx == OD_INDEX_MAGNETS){
        switch(subidx) {
            case MAGNET_SUBIDX_CURRENT :
                *((float *)buf) = cmd_magnet_current_get();
                break;
            case MAGNET_SUBIDX_CURRENT_FACTORED :
                *((uint16_t *)buf) = cmd_magnet_current_factored_get();
                break;
            case MAGNET_SUBIDX_STATE :
                *((uint8_t *)buf) = cmd_magnet_state_get();
                break;
            case MAGNET_SUBIDX_COUNT :
                *((uint32_t *)buf) = MAGNET_SUBIDX_EOL-1;
                break;
            default:
                read_err = __LINE__; // Not implemented
        }
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/**
 * Size callback for user object.  The CAN Open stack needs to know the size
 * of a float.   There if if the OD entry has one object it needs to know the
 * size of, it needs to this function for all sub-indexes.
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return size of the field in bytes
 */
static uint32_t MagnetSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    switch(subidx) {
        case MAGNET_SUBIDX_CURRENT :
            size = sizeof(((control_magnets_t *)0)->current);
            break;
        case MAGNET_SUBIDX_CURRENT_FACTORED :
            size = sizeof(((control_magnets_t *)0)->current_factored);
            break;
        case MAGNET_SUBIDX_STATE :
            size = sizeof(((control_magnets_t *)0)->status.state);
            break;
        case MAGNET_SUBIDX_COUNT :
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
void MagnetsOD(OD_DYN *self)
{
    /* Magnet Control */
    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS, MAGNET_SUBIDX_COUNT,
            CO_UNSIGNED16|CO_OBJ____R_), CO_TMAGNET, (uintptr_t)&MagnetType);
    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS, MAGNET_SUBIDX_CURRENT,
            CO_FLOAT     |CO_OBJ____RW), CO_TMAGNET, (uintptr_t)&MagnetType);
    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS, MAGNET_SUBIDX_CURRENT_FACTORED,
            CO_UNSIGNED16|CO_OBJ____R_), CO_TMAGNET, (uintptr_t)&MagnetType);
    ODAdd(self, CO_KEY(OD_INDEX_MAGNETS, MAGNET_SUBIDX_STATE,
            CO_UNSIGNED8 |CO_OBJ____RW), CO_TMAGNET, (uintptr_t)&MagnetType);
    
    magnet_obj_add_telemetry(self);
}

/* *****************************************************************************
 End of File
 */
