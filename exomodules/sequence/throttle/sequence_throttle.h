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

#ifndef THRUSTER_CONTROL_HALO12TEST_THRUSTER_CONTROL_MODULES_SEQUENCE_THROTTLE_SEQUENCE_THROTTLE_H_
#define THRUSTER_CONTROL_HALO12TEST_THRUSTER_CONTROL_MODULES_SEQUENCE_THROTTLE_SEQUENCE_THROTTLE_H_
#include "sequence/sequence/control_sequence.h"

typedef enum {
  THROTTLE_SEQ_1,
  THROTTLE_SEQ_2,
  THROTTLE_SEQ_3,
  THROTTLE_SEQ_4,
  THROTTLE_SEQ_5,
  THROTTLE_SEQ_6,
  THROTTLE_SEQ_COUNT,
} throttle_sequence_t;

typedef struct sequence_throttle {
  sequence_array_t *seq;
  char *name;
} sequence_throttle_t;

#endif //THRUSTER_CONTROL_HALO12TEST_THRUSTER_CONTROL_MODULES_SEQUENCE_THROTTLE_SEQUENCE_THROTTLE_H_
