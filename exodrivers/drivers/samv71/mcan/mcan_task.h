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
 * @Company
 *      Exoterra
 * @File
 *      mcan_task.h
 * @Summary
 *      MCAN interface functions
 * @Description
 *      The interface between the application and the MCAN interface.
 * 
 */

#ifndef _MCAN_TASK_H    /* Guard against multiple inclusion */
#define _MCAN_TASK_H

/* ************************************************************************** */
/* ************************************************************************** */
/* Section: Included Files                                                    */
/* ************************************************************************** */
/* ************************************************************************** */
#include "hal.h"
#include "binary_range_tree.h" // Binary Range Tree

typedef enum{
    COMMS_SERIAL_SUBMODULE,
    COMMS_CAN_SUBMODULE,
} COMMS_SUBMODULES;

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif


typedef int (*CAN_MSG_CALLBACK) (uint16_t id, uint8_t *data, can_dlc_t dlc);
typedef struct can_msg_callback {
    range_node_t node;
    CAN_MSG_CALLBACK cb;
} mcan_msg_callback_t;

/* BUS Number ENUM used to specify with which bus to use for which interface */
typedef enum {
    CAN_BUS_0,
    CAN_BUS_1,
} CAN_BUS_INTERFACE;

/* Structure to specify which is the external & client bus */
typedef struct {
    /* external is the bus where commands to us come from */
    CAN_BUS_INTERFACE external;
    /* client is the bus where we send command to the things we control (Atmegas) */
    CAN_BUS_INTERFACE client;
} can_bus_setup_t;

void mcan_task_init(can_bus_setup_t *setup);
int mcan_send_sync_client(void);
int mcan_send_msg_client(uint16_t cobid, uint8_t *data, can_dlc_t dlc);
int mcan_send_msg_extern(uint32_t cobid, uint8_t *data, can_dlc_t dl);
void mcan_task_register_cb(msg_callback_t *cb_node);
int mcan_ext_msg_rcv(uint32_t *identifier, uint8_t *data, uint8_t *DLC, int block);

    /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _MCAN_TASK_H */

/* *****************************************************************************
 End of File
 */
