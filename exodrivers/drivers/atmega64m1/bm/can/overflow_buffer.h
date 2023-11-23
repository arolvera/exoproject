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

/* 
 * File:   overflow_buffer.h
 * Author: jmeyers
 *
 * Created on March 29, 2022, 5:13 AM
 */
#include <stdint.h>
#ifndef OVERFLOW_BUFFER_H
#define	OVERFLOW_BUFFER_H

#ifdef	__cplusplus
extern "C" {
#endif

int  ofb_message_get(uint16_t *id, uint8_t *dlc, uint8_t *data);
int  ofb_message_save(uint16_t  id, uint8_t  dlc, uint8_t *data);
bool ofb_full(void);
bool ofb_empty(void);

#ifdef	__cplusplus
}
#endif

#endif	/* NEWFILE_H */

