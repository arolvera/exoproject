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
 * anode_object.c
 *
 * @Company Exoterra
 * @File anode_object.c
 * @Summary  User object for anode control.
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

#include "anode_object.h"                           // Header for this module
#include "anode/control_anode.h"           // Anode control module
#include "thruster_control.h"
#include "user_object_od_indexes.h"

#include "hsi_memory.h"
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

static CO_ERR   AnodeWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static CO_ERR   AnodeRead (struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);
static uint32_t AnodeSize (struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width);

/**
 * Anode Control User Object
 */
CO_OBJ_TYPE AnodeType = {
    AnodeSize,    /* type function to get object size      */
    0,            /* type function to control type object  */
    AnodeRead,    /* type function to read object content  */
    AnodeWrite,   /* type function to write object content */
};
#define CO_TANODE  ((CO_OBJ_TYPE*)&AnodeType)

/* SUB-Indexes - Must match this order in the Object Dictionary */
typedef enum {
  ANODE_SUBIDX_COUNT,
  ANODE_SUBIDX_VOLTS,
  ANODE_SUBIDX_VOLTS_FACTORED,
  ANODE_SUBIDX_CUR,
  ANODE_SUBIDX_CUR_FACTORED,
  ANODE_SUBIDX_POWER_SUPPLY,
  ANODE_SUBIDX_EOL,
} OD_ANODE;


typedef enum{
  ANODE_DIAG_SUBIDX_COUNT,
  ANODE_DIAG_SUBIDX_VX,
  ANODE_DIAG_SUBIDX_VY,
  ANODE_DIAG_SUBIDX_VOUT,
  ANODE_DIAG_SUBIDX_IOUT,
  ANODE_DIAG_SUBIDX_CURR_STATE,
  ANODE_DIAG_SUBIDX_ERR_CODE,
  ANODE_DIAG_SUBIDX_X_PWM_OUT,
  ANODE_DIAG_SUBIDX_Y_PWM_OUT,
  ANODE_DIAG_SUBIDX_MODE,
  ANODE_DIAG_SUBIDX_TEMP,
  ANODE_DIAG_SUBIDX_RAW_INPUT_V,
  ANODE_DIAG_SUBIDX_FILT_INPUT_V,
  ANODE_DIAG_SUBIDX_EOL,
}OD_ANODE_DIAG;


/**
 * @brief Write to the anode variables file
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size of the data pointed at by buffer
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR AnodeWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);

    switch(subidx) {
        case ANODE_SUBIDX_VOLTS :
            write_err = ctrl_anode_volts_set(*((float *)buf));
            break;
        case ANODE_SUBIDX_CUR   :
            write_err = ctrl_anode_cur_set(*((float *)buf));
            break;
        case ANODE_SUBIDX_POWER_SUPPLY :
            write_err = ctrl_anode_ps_state_set(*((uint8_t *)buf));
            break;
        case ANODE_SUBIDX_COUNT : /* Read-only - fall through to error */
        case ANODE_SUBIDX_VOLTS_FACTORED:
        case ANODE_SUBIDX_CUR_FACTORED:
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}

/**
 * @brief Write to the anode variables file.
 * @param obj object dictionary info
 * @param node CO Node info
 * @param buf buffer to write from
 * @param size size of the data pointed at by buffer
 * @return on err CO_ERR_TYPE_WR, for success CO_ERR_NONE
 */
static CO_ERR AnodeRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    uint16_t idx    = CO_GET_IDX(obj->Key);

    if(idx == OD_INDEX_ANODE){
        switch(subidx) {
            case ANODE_SUBIDX_VOLTS :
                *((float *)buf) = ctrl_anode_volts_get();
                break;
            case ANODE_SUBIDX_VOLTS_FACTORED :
                *((uint16_t *)buf) = ctrl_anode_volts_factored_get();
                break;
            case ANODE_SUBIDX_CUR :
                *((float *)buf) = ctrl_anode_cur_get();
                break;
            case ANODE_SUBIDX_CUR_FACTORED :
                *((uint16_t *)buf) = ctrl_anode_cur_factored_get();
                break;
            case ANODE_SUBIDX_POWER_SUPPLY :
                *((uint8_t *)buf) = ctrl_anode_ps_state_get();
                break;
            case ANODE_SUBIDX_COUNT :
                *((uint32_t *)buf) = ANODE_SUBIDX_EOL-1;
                break;
            default:
                read_err = __LINE__; // Not implemented
        }
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}

/**
 * @brief Size callback for the Anode user object. The CAN Open stack needs to know the
 * size of each object.
 * @param obj object dictionary info
 * @param node CO Node info
 * @param width
 * @return size of the field in bytes
 */
static uint32_t AnodeSize(struct CO_OBJ_T *obj, struct CO_NODE_T *node, uint32_t width)
{
    uint32_t size = 0;
    uint8_t  subidx = CO_GET_SUB(obj->Key);
    switch(subidx) {
        case ANODE_SUBIDX_VOLTS :
            size = sizeof(((control_anode_t *)0)->voltage);
            break;
        case ANODE_SUBIDX_VOLTS_FACTORED :
            size = sizeof(((control_anode_t *)0)->voltage_factored);
            break;
        case ANODE_SUBIDX_CUR :
            size = sizeof(((control_anode_t *)0)->current);
            break;
        case ANODE_SUBIDX_CUR_FACTORED :
            size = sizeof(((control_anode_t *)0)->current_factored);
            break;
        case ANODE_SUBIDX_POWER_SUPPLY :
            size = sizeof(((control_anode_t *)0)->ps_state);
            break;
        case ANODE_SUBIDX_COUNT :
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
void AnodeOD(OD_DYN *self)
{
    /* Anode Control */
    ODAdd(self, CO_KEY(OD_INDEX_ANODE, ANODE_SUBIDX_COUNT,
                       CO_UNSIGNED16|CO_OBJ____R_), CO_TANODE, (uintptr_t)&AnodeType);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE, ANODE_SUBIDX_VOLTS,
                       CO_FLOAT     |CO_OBJ____RW), CO_TANODE, (uintptr_t)&AnodeType);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE, ANODE_SUBIDX_VOLTS_FACTORED,
                       CO_UNSIGNED16|CO_OBJ____R_), CO_TANODE, (uintptr_t)&AnodeType);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE, ANODE_SUBIDX_CUR,
                       CO_FLOAT     |CO_OBJ____RW), CO_TANODE, (uintptr_t)&AnodeType);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE, ANODE_SUBIDX_CUR_FACTORED,
                       CO_UNSIGNED16|CO_OBJ____R_), CO_TANODE, (uintptr_t)&AnodeType);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE, ANODE_SUBIDX_POWER_SUPPLY,
                       CO_UNSIGNED8 |CO_OBJ____RW), CO_TANODE, (uintptr_t)&AnodeType);


    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_COUNT, CO_UNSIGNED8  |CO_OBJ_D__R_), 0, ANODE_DIAG_SUBIDX_EOL - 1);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_VX, CO_UNSIGNED32 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.v_x);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_VY, CO_UNSIGNED32 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.v_y);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_VOUT, CO_UNSIGNED32 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.vout);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_IOUT, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.iout);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_CURR_STATE, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.current_state);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_ERR_CODE, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.error_code);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_X_PWM_OUT, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.x_pwm_output);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_Y_PWM_OUT, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.y_pwn_output);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_MODE, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.mode);

    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_TEMP, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.temperature);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_RAW_INPUT_V, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.raw_input_voltage);
    ODAdd(self, CO_KEY(OD_INDEX_ANODE_DIAG, ANODE_DIAG_SUBIDX_FILT_INPUT_V, CO_UNSIGNED16 | CO_OBJ____R_),
          0, (uintptr_t)&hsi_mem.anode_telem.filtered_input_voltage);
}
/* *****************************************************************************
 End of File
 */
