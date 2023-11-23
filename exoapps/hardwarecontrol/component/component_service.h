/*
           Copyright (C) 2022 ExoTerra Corp - All Rights Reserved

 Unauthorized copying of this file via any medium is strictly prohibited.
 Proprietary and confidential.  Any unauthorized use, duplication, transmission,
 distribution, or disclosure of this software is expressly forbidden.

 This Copyright notice may not be removed or modified without prior written
 consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this 
 software without notice.

 ExoTerra Corp
 7640 S. Alkire Pl.
 Littleton, CO 80127
 USA

 Voice:  +1 (720) 788-2010
 http:   www.exoterracorp.com
 email:  contact@exoterracorp.com
*/

#ifndef COMMON_H
#define	COMMON_H

#include <stddef.h>
#include <stdbool.h>
#include "operations.h"

uint8_t cs_id_get(void);
void cs_service(uint8_t ops_id);
void cs_init(uint8_t ops_id);
states_t cs_init_complete(uint8_t ops_id);
void cs_invalidate_img(uint8_t* msg);
void cs_reset_proc(uint8_t* arg);

#endif	/* COMMON_H */

