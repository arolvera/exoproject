/* 
 * File:   commsHandler_M64.h
 * Author: fnorwood
 *
 * Created on March 2, 2022, 2:45 PM
 */

#ifndef COMMSHANDLER_M64_H
#define	COMMSHANDLER_M64_H

#include "BuckControl.h"

void buck_sync(uint32_t id, uint8_t dlc, volatile uint8_t *fifo);
void CANcommHandler(void);
void Send_SAM_reply(buck_state_t State, uint8_t Status, uint16_t adc);

// Structure for automatically setting the controller ID based on HW PC1 pin
typedef struct{
    uint32_t Responce;
    uint32_t Command;
    uint32_t Broadcast;
    uint32_t Health;
    uint32_t MyAddress;
}MyCan_ID_t;

// Structure for command reply message
typedef struct{
    uint8_t CommandEcho;
}CommandReply_t;

#endif	/* COMMSHANDLER_M64_H */

