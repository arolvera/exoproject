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
 * valves_object.c
 *
 * @Company Exoterra
 * @File update_object.c
 * @Summary  User object for valves control.
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
#include <string.h>                 // memcpy

#include "valves_object.h"                           // Header for this module
#include "valve/control_valves.h"           // Value control module
#include "health/health.h"
#include "thruster_control.h"
#include "user_object_od_indexes.h"
#include "hsi_memory.h"
/* ************************************************************************** */
/* ************************************************************************** */
/* Section: File Scope or Global Data                                         */
/* ************************************************************************** */
/* ************************************************************************** */


//static uint16_t* health_history_buf;

static CO_ERR ValvesWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR ValvesRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t ValvesSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);


/** 
 * Valve Control User Object
 */
CO_OBJ_TYPE ValvesType = {
    ValvesSize,    /* type function to get object size      */
    0,             /* type function to control type object  */
    ValvesRead,    /* type function to read object content  */
    ValvesWrite,   /* type function to write object content */
};
#define CO_TVALVE  ((CO_OBJ_TYPE*)&ValvesType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
    VALVES_SUBIDX_COUNT = 0,
    VALVES_SUBIDX_CATHODE_HF, /* High Flow */
    VALVES_SUBIDX_CATHODE_HF_FACTORED,
    VALVES_SUBIDX_CATHODE_LF, /* Low Flow */
    VALVES_SUBIDX_CATHODE_LF_FACTORED,
    VALVES_SUBIDX_ANODE_FLOW,
    VALVES_SUBIDX_ANODE_FLOW_FACTORED,
    VALVES_SUBIDX_LATCH_VALVE,
    VALVES_SUBIDX_EOL,
} od_valves_t;

typedef enum{
    VALVES_DIAG_SUBIDX_COUNT,
    VALVES_DIAG_SUBIDX_ANODE_V,
    VALVES_DIAG_SUBIDX_CATHODE_HF_V,
    VALVES_DIAG_SUBIDX_CATHODE_LF_V,
    VALVES_DIAG_SUBIDX_TEMP_C,
    VALVES_DIAG_SUBIDX_TANK_PRESSURE,
    VALVES_DIAG_SUBIDX_CATHODE_PRESSURE,
    VALVES_DIAG_SUBIDX_ANODE_PRESSURE,
    VALVES_DIAG_SUBIDX_REGULATOR_PRESSURE,
    VALVES_DIAG_SUBIDX_MSG_CNT,
    VALVES_DIAG_SUBIDX_CAN_ERR,
    VALVES_DIAG_SUBIDX_EOL,
}OD_VALVES_DIAG;

/* ************************************************************************** */
/* ************************************************************************** */
// Section: Local Functions                                                   */
/* ************************************************************************** */
/* ************************************************************************** */
/**
 * Write to the valves variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR ValvesWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    od_valves_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case VALVES_SUBIDX_CATHODE_HF:
            write_err = ctrl_valves_cathode_hf_set(*(float *)buf);
            break;
        case VALVES_SUBIDX_CATHODE_LF:
            write_err = ctrl_valves_cathode_lf_set(*(float *)buf);
            break;
        case VALVES_SUBIDX_ANODE_FLOW:
            write_err = ctrl_valves_anode_flow_set(*(float *)buf);
            break;
        case VALVES_SUBIDX_LATCH_VALVE:
            write_err = ctrl_valves_latch_valve_set(*(uint8_t*)buf);
            break;
        case VALVES_SUBIDX_COUNT : /* Read-only - fall through to error */
        case VALVES_SUBIDX_ANODE_FLOW_FACTORED :
        case VALVES_SUBIDX_CATHODE_HF_FACTORED :
        case VALVES_SUBIDX_CATHODE_LF_FACTORED :
        default:
           write_err = __LINE__; //
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * Write to the valves variables file
 * 
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR ValvesRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    od_valves_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case VALVES_SUBIDX_CATHODE_HF:
            *((float *)buf) = ctrl_valves_cathode_hf_get();
            break;
        case VALVES_SUBIDX_CATHODE_HF_FACTORED:
            *((uint16_t *)buf) = ctrl_valves_cathode_hf_factored_get();
            break;
        case VALVES_SUBIDX_CATHODE_LF:
            *((float *)buf) = ctrl_valves_cathode_lf_get();
            break;
        case VALVES_SUBIDX_CATHODE_LF_FACTORED:
            *((uint16_t *)buf) = ctrl_valves_cathode_lf_factored_get();
            break;
        case VALVES_SUBIDX_ANODE_FLOW:
            *((float *)buf) = ctrl_valves_anode_flow_get();
            break;
        case VALVES_SUBIDX_ANODE_FLOW_FACTORED:
            *((uint16_t *)buf) = ctrl_valves_anode_flow_factored_get();
            break;
        case VALVES_SUBIDX_LATCH_VALVE:
            *((uint8_t *)buf) = ctrl_valves_latch_valve_get();
            break;
        case VALVES_SUBIDX_COUNT : 
            *((uint8_t *)buf) = VALVES_SUBIDX_EOL - 1;
            break;
        default:
            read_err = __LINE__; // Not implemented
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
static uint32_t ValvesSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    
    od_valves_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case VALVES_SUBIDX_COUNT :
            size = sizeof(uint8_t);
            break;
        case VALVES_SUBIDX_CATHODE_HF :
            size = sizeof(((control_valves_t *)0)->cathode_hf);
            break;
        case VALVES_SUBIDX_CATHODE_HF_FACTORED :
            size = sizeof(((control_valves_t *)0)->cathode_hf_factored);
            break;
        case VALVES_SUBIDX_CATHODE_LF :
            size = sizeof(((control_valves_t *)0)->cathode_lf);
            break;
        case VALVES_SUBIDX_CATHODE_LF_FACTORED :
            size = sizeof(((control_valves_t *)0)->cathode_lf_factored);
            break;
        case VALVES_SUBIDX_ANODE_FLOW :
            size = sizeof(((control_valves_t *)0)->anode_flow);
            break;
        case VALVES_SUBIDX_ANODE_FLOW_FACTORED :
            size = sizeof(((control_valves_t *)0)->anode_flow_factored);
            break;
        case VALVES_SUBIDX_LATCH_VALVE :
            size = sizeof(((control_valves_t *)0)->latch_valve);
            break;
        case VALVES_SUBIDX_EOL :
        default:
            size = 0;
            
    }
    return size;
}
/* ************************************************************************** */
/* ************************************************************************** */
// Section: Interface Functions                                               */
/* ************************************************************************** */
/* ************************************************************************** */

void ValvesOD(OD_DYN *self)
{
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_COUNT,
            CO_UNSIGNED16|CO_OBJ____R_), CO_TVALVE,(uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_CATHODE_HF,
            CO_FLOAT     |CO_OBJ____RW), CO_TVALVE, (uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_CATHODE_HF_FACTORED,
            CO_UNSIGNED16|CO_OBJ____R_), CO_TVALVE, (uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_CATHODE_LF,
            CO_FLOAT     |CO_OBJ____RW), CO_TVALVE, (uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_CATHODE_LF_FACTORED,
            CO_UNSIGNED16|CO_OBJ____R_), CO_TVALVE, (uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_ANODE_FLOW,
            CO_FLOAT     |CO_OBJ____RW), CO_TVALVE, (uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_ANODE_FLOW_FACTORED,
            CO_UNSIGNED16|CO_OBJ____R_), CO_TVALVE, (uintptr_t)&ValvesType);
    ODAdd(self, CO_KEY(OD_INDEX_VALVES, VALVES_SUBIDX_LATCH_VALVE,
            CO_UNSIGNED8 |CO_OBJ____RW), CO_TVALVE, (uintptr_t)&ValvesType);

    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_COUNT, CO_UNSIGNED8  |CO_OBJ_D__R_), 
                0, VALVES_DIAG_SUBIDX_EOL - 1);

    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_ANODE_V, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_1.anode_v);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_CATHODE_HF_V, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_1.cathode_hf_v);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_CATHODE_LF_V, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_1.cathode_lf_v);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_TEMP_C, CO_SIGNED32 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_1.temperature);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_TANK_PRESSURE, CO_UNSIGNED32 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_2.tank_pressure);

    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_CATHODE_PRESSURE, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_2.cathode_pressure);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_ANODE_PRESSURE, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_2.anode_pressure);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_REGULATOR_PRESSURE, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_2.regulator_pressure);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_MSG_CNT, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_3.can_msg_cnt);
    
    ODAdd(self, CO_KEY(OD_INDEX_VALVES_DIAG, VALVES_DIAG_SUBIDX_CAN_ERR, CO_UNSIGNED16 | CO_OBJ____R_),
                0, (uintptr_t)&hsi_mem.valves_telem_3.can_err);
}
/* *****************************************************************************
 End of File
 */
