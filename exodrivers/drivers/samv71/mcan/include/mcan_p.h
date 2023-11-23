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
#ifndef _MCAN_PRIVATE_H    /* Guard against multiple inclusion */
#define _MCAN_PRIVATE_H
#include "definitions.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

// The Sync-Producer provides the synchronization-signal for the Sync-Consumer.
//	When the Sync-Consumer receive the signal they start carrying out
//	their synchronous tasks.  The SAMV71 can be a producer or consumer
#define MSG_SYNC_ID         0x80
#define MSG_SYNC_LENGTH        1
    
typedef struct MailboxInfoTag
{
	uint32_t    id;
	uint8_t     length;
	uint16_t    timestamp;
    MCAN_MSG_RX_FRAME_ATTRIBUTE   attr;
} MailboxInfoType;

typedef struct MailBox8Tag
{
	MailboxInfoType info;
	uint8_t         data[8];
} Mailbox8Type;

typedef struct MailBox12Tag
{
	MailboxInfoType info;
	uint8_t         data[12];
} Mailbox12Type;

typedef struct MailBox16Tag
{
	MailboxInfoType info;
	uint8_t         data[16];
} Mailbox16Type;

typedef struct MailBox20Tag
{
	MailboxInfoType info;
	uint8_t         data[20];
} Mailbox20Type;

typedef struct MailBox24Tag
{
	MailboxInfoType info;
	uint8_t         data[24];
} Mailbox24Type;

typedef struct MailBox32Tag
{
	MailboxInfoType info;
	uint8_t         data[32];
} Mailbox32ype;

typedef struct MailBox48Tag
{
	MailboxInfoType info;
	uint8_t         data[48];
} Mailbox48Type;

typedef struct MailBox64Tag
{
	MailboxInfoType info;
	uint8_t         data[64];
} Mailbox64Type;

/* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _MCAN_PRIVATE_H */
