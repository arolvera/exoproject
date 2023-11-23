/**
 * @file    client_update_server.h
 *
 * @brief   This module does the work of updating the client.
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

#ifndef CLIENT_UPDATE_SERVER_H
#define CLIENT_UPDATE_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Intialize tasks and queues.  This will spin up a task that waits on a command
 * queue.   Responses are handled through a callback function that comes in the
 * command queue.
 * @return 0 on success, error otherwise (always successfully for now)
 */
int cus_init(void);


/**
 * Alert the update server that a client requested a new image
 */
int cus_image_request(int client_id);


/**
 * Prepare the clients to accept a new image
 */
int cus_client_prepare(int client_id);

#ifdef __cplusplus
}
#endif

#endif // CLIENT_UPDATE_SERVER_H
