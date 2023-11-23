/**
 * @file    thruster_command_object.c
 *
 * @brief   Implementation for thruster command CANOpen objects for Halo12.
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

#include "client-control/client_control.h"
#include "client-control/control_autostart.h"
#include "user_object_od_indexes.h"
#include "thruster_command_object.h"
#include "thruster-start/control_thruster_start.h"
#include "throttle/control_throttle.h"
#include "conditioning/control_condition.h"
#include "bit/control_bit.h"



static CO_ERR   ThrusterCommandWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   ThrusterCommandRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t ThrusterCommandSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);



/** 
 * @brief   ThrusterCommand Control User Object
 */
CO_OBJ_TYPE ThrusterCommandType = {
    ThrusterCommandSize,    /* type function to get object size      */
    0,                      /* type function to control type object  */
    ThrusterCommandRead,    /* type function to read object content  */
    ThrusterCommandWrite,   /* type function to write object content */
};
#define CO_TTHRUSTER_COMMAND ((CO_OBJ_TYPE*)&ThrusterCommandType)



void ThrusterCommandOD(OD_DYN *self)
{
    /* ThrusterCommand Control */
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_COUNT,
                CO_UNSIGNED8 |CO_OBJ____R_), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_READY_MODE,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_STEADY_STATE,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_SHTDN,
                CO_UNSIGNED8 |CO_OBJ_____W), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_THRUST,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_STAT,
                CO_UNSIGNED32|CO_OBJ____R_), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_CONDITION,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_TEST_BIT,
                CO_UNSIGNED32|CO_OBJ____RW), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
    ODAdd(self, CO_KEY(OD_INDEX_THRUSTER_COMMAND, THRUSTER_COMMAND_SUBIDX_START,
                CO_UNSIGNED32|CO_OBJ_____W), CO_TTHRUSTER_COMMAND, (uintptr_t)&ThrusterCommandType);
}

/**
 * @brief   Process a command received on the CANOpen port.
 * 
 * @param   obj object dictionary info
 * @param   node CO Node info
 * @param   buf buffer to write from
 * @param   size size to write
 *
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR ThrusterCommandWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;

    /**
     * Halo 6 only defined one parameter called command that commanded the thruster on or off.
     * Halo 12 requires a second parameter to set the first setpoint after ignition. The cmd
     * variable is renamed to params and a new cmd variable is created that takes the first
     * byte of the data. ANother new variable is created called sp that takes the second byte.
     * In the future, more parameters may be defined.
     */
    uint32_t params = (*((uint32_t*)buf));
    uint8_t cmd = (uint8_t)(params & 0x000000FF);
    uint8_t sp = (uint8_t)((params & 0x0000FF00) >> 8);
    
    uint8_t subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case THRUSTER_COMMAND_SUBIDX_READY_MODE:
            if(cmd == THRUSTER_COMMAND_OFF) {
                ctrl_sequence_abort();
            } else if(cmd == THRUSTER_COMMAND_ON) {
                write_err = ctrl_ts_ready_mode_run();
            } else {
                write_err = __LINE__;
            }
            break;
        case THRUSTER_COMMAND_SUBIDX_STEADY_STATE:
            if(cmd == THRUSTER_COMMAND_OFF) {
                ctrl_sequence_abort();
            } else if(cmd == THRUSTER_COMMAND_ON) {
                write_err = ctrl_ts_steady_state_run(sp);
            } else {
                write_err = __LINE__;
            }
            break;
        case THRUSTER_COMMAND_SUBIDX_SHTDN:
            ctrl_autostop();
            client_shutdown();
            break;
        case THRUSTER_COMMAND_SUBIDX_THRUST:
            write_err = ctrl_throttle_setpoint_set(cmd);
            break;
        case THRUSTER_COMMAND_SUBIDX_CONDITION:
            if(cmd) {
                write_err = ctrl_condition_start(cmd);
            } else {
                write_err = ctrl_condition_abort();
            }
            break;
        case THRUSTER_COMMAND_SUBIDX_TEST_BIT:
            if(cmd) {
                write_err = ctrl_bit_run(cmd);
            } else {
                ctrl_sequence_abort();
            }
            break;
        case THRUSTER_COMMAND_SUBIDX_START:
            write_err = ctrl_autostart(cmd);
            break;
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * @brief   Get the current thruster command state.
 * 
 * @param   obj object dictionary info
 * @param   node CO Node info
 * @param   buf buffer to write from
 * @param   size size to write
 *
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR ThrusterCommandRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    
    switch(subidx) {
        case THRUSTER_COMMAND_SUBIDX_COUNT:
            *((uint8_t *)buf) = THRUSTER_COMMAND_SUBIDX_EOL;
            break;
        case THRUSTER_COMMAND_SUBIDX_READY_MODE:
            *((uint32_t *)buf) = ctrl_ts_ready_mode_stat();
            break;
        case THRUSTER_COMMAND_SUBIDX_STEADY_STATE:
            *((uint32_t *)buf) = ctrl_ts_steady_state_stat();
            break;
        case THRUSTER_COMMAND_SUBIDX_THRUST:
            *((uint32_t *)buf) = (ctrl_sequence_setpoint_get() + 1);
            break;
        case THRUSTER_COMMAND_SUBIDX_STAT:
            *((uint32_t *)buf) = Thruster_Control_State;
            break;
        case THRUSTER_COMMAND_SUBIDX_CONDITION:
            *((uint32_t *)buf) = ctrl_condition_stat();
            break;
        case THRUSTER_COMMAND_SUBIDX_TEST_BIT:
            *((uint32_t *)buf) = ctrl_bit_stat();
            break;
        default:
            read_err = __LINE__; // Not implemented
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/**
 * @brief   Size callback for user object. The CAN Open stack needs to know the size of each object.
 *
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size to write
 *
 * @return size of the field in bytes
 */
static uint32_t ThrusterCommandSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    switch(subidx) {
        case THRUSTER_COMMAND_SUBIDX_COUNT:
        case THRUSTER_COMMAND_SUBIDX_SHTDN:
            size = sizeof(uint8_t);
            break;
        case THRUSTER_COMMAND_SUBIDX_STEADY_STATE:
        case THRUSTER_COMMAND_SUBIDX_READY_MODE:
        case THRUSTER_COMMAND_SUBIDX_THRUST:
        case THRUSTER_COMMAND_SUBIDX_STAT:
        case THRUSTER_COMMAND_SUBIDX_CONDITION:
        case THRUSTER_COMMAND_SUBIDX_START:
        case THRUSTER_COMMAND_SUBIDX_TEST_BIT:
            size = sizeof(uint32_t);
            break;
        default:
            size = 0; // Not implemented
    }
    return size;
}
