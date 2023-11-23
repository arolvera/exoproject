/**
 * @file    client_service.h
 *
 * @brief   ??? Random lists of services for client control. How do the services work?
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

#ifndef CLIENT_SERVICE_H
#define CLIENT_SERVICE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "client_p.h"
#include "portmacro.h"

/* Callback types for servicing clients */
typedef int (*CLIENT_SERVICE_CALLBACK) (void *params);
typedef struct client_service {
    CLIENT_SERVICE_CALLBACK cb;
    void *params;
} client_service_t;

/**
 * @brief
 * @param service_queue
 */
void client_service_init(QueueHandle_t *service_queue);

/**
 * @brief
 * @param parm
 * @return
 */
int ctrl_sequence_shutdown(void *parm);

/**
 * @brief This function will power up or down the clients and will start
 * or stop the keeper task on the ECPK.
 * @param power_on True = power on clients and start keeper task
 *                 False = power down and stop
 * @note This function cannot be called with early versions of PPU hardware
 * such as the Astranis EDU unit. For these early boards we must use
 * client_reset() defined below. If and when the hardware is changed to
 * allow powering the boards on/off, we will revisit this.
 */
void client_power_state_set(bool power_on);

/**
 * @brief This function send a command to do a soft reset on the clients
 * and the keeper task. It is created as a work around for the hardware
 * issue described in the client_power_state_set() comments.
 */
void client_reset(void);

/**
 * @brief When the client is reset, a power state variable is set to false
 * to block comms with the clients. After they complete reset, this state
 * flag needs to be set to true to unblock comms. This will be called from
 * client_booted_register_component() after all components have booted.
 */
void client_reset_complete(void);

/**
 * @brief
 * @param service
 * @return
 */
BaseType_t client_service_queue(client_service_t *service);

/**
 * @brief Function to queue power state changes to be handled in the client
 * service queue.
 *
 * @param power state to change to
 *  * @return void
 */
void client_power_queue(bool power_on);

/**
 * @brief Queue a command to reset the clients. This will cancel all sequences
 * and put the thruster state back to Transition to Standby.
 */
void client_reset_queue(void);

/**
 * @brief
 */
void client_shutdown(void);



#ifdef	__cplusplus
}
#endif

#endif //CLIENT_SERVICE_H
