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
#include <stddef.h>
#include <stdbool.h>

#include "overflow_buffer.h"

#define MAX_DATA_SIZE 8
typedef struct _overflow_message {
    uint16_t id;
    uint8_t  dlc;
    uint8_t  data[MAX_DATA_SIZE];
} overflow_message_t;

/*
 * Circular Overflow buffer
 * 
 * Empty when head == tail
 * Full when head+1 == tail
 * 
 */
#define OVERFLOW_BUFFER_SIZE 32       /* Must be a power of two */
#define ADVANCE_MARK(__X__) ((__X__ + 1) & (OVERFLOW_BUFFER_SIZE - 1))
#define ADVANCE_HEAD() ADVANCE_MARK(ofb_head)
#define ADVANCE_TAIL() ADVANCE_MARK(ofb_tail)
volatile int ofb_head = 0;
volatile int ofb_tail = 0;
static volatile overflow_message_t overflow_buffer[OVERFLOW_BUFFER_SIZE];


bool ofb_full(void)
{
    return  ADVANCE_HEAD() == ofb_tail;
}

bool ofb_empty(void)
{
    return ofb_head == ofb_tail;
}
/**
 * Get the next message in the buffer
 * @param id pointer to store id
 * @param dlc pointer to store data length
 * @param data pointer to store data (must be at least 8 bytes)
 * @return 0 if a message was available, non-zero otherwise
 */
int  ofb_message_get(uint16_t *id, uint8_t *dlc, uint8_t *data)
{
    int err = 0;
    if(ofb_empty()) {
       err = __LINE__; 
    } else {
        volatile overflow_message_t *msg = &overflow_buffer[ofb_tail];
        *id = msg->id;
        *dlc = msg->dlc;
        for(int i = 0; i < *dlc && i < MAX_DATA_SIZE; i++) {
            data[i] = msg->data[i];
        }
        ofb_tail = ADVANCE_TAIL();
    }
    return err;
}
/**
 * Save message into overflow buffer
 * @param id message id
 * @param dlc data length of message
 * @param data message data
 * @return 0 on success, non-zero if data is full
 */
int  ofb_message_save(uint16_t id, uint8_t dlc, uint8_t *data)
{
    int err = 0;
    if(ofb_full()) {
       err = __LINE__; 
    } else {
        volatile overflow_message_t *msg = &overflow_buffer[ofb_head];
        msg->id = id;
        msg->dlc = dlc;
        for(int i = 0; i < dlc && i < MAX_DATA_SIZE; i++) {
            msg->data[i] = data[i];
        }
        ofb_head = ADVANCE_HEAD();
    }
    return err;
}
