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

#ifndef HALO12_EXODRIVERS_DRIVER_VA41630_BM_CAN_DRIVER_CAN_VA41630_H_
#define HALO12_EXODRIVERS_DRIVER_VA41630_BM_CAN_DRIVER_CAN_VA41630_H_
#include "can/hal_can.h"

int can_notify_tx_avail_BM(void* arg);

int can_notify_rx_task_BM(void);
int can_notif_tx_buff_avail_BM(void*);
int can_queue_rx_message_BM(int handle, message_t* msg);
void can_rx_service(const int handle);
int can_notify_rx_task_BM(void);
uint32_t can_rx_is_empty(int handle);



#endif //HALO12_EXODRIVERS_DRIVER_VA41630_BM_CAN_DRIVER_CAN_VA41630_H_
