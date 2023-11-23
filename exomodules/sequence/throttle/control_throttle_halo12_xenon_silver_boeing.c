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

#ifndef THRUSTER_CONTROL_HALO12TEST_THRUSTER_CONTROL_MODULES_SEQUENCE_THROTTLE_CONTROL_THROTTLE_SILVER_HALO_H_
#define THRUSTER_CONTROL_HALO12TEST_THRUSTER_CONTROL_MODULES_SEQUENCE_THROTTLE_CONTROL_THROTTLE_SILVER_HALO_H_

#include "sequence_throttle.h"
#define DECLARE_GLOBALS
#include "ext_decl_define.h"

#define NAME_THROTTLE_SEQUENCE_1 "throttle 1"
static sequence_array_t sequence_throttle_1[SEQUENCE_MAX_STEPS_THROTTLE] = {
    0x020B060100000000,
    0x0506060100000000,
    0x030B060100000000,
    0x020C060100000000,
    0, /* EOL */
};
#define NAME_THROTTLE_SEQUENCE_2 "throttle 2"
static sequence_array_t sequence_throttle_2[SEQUENCE_MAX_STEPS_THROTTLE] = {
    0x020B060100000000,
    0x030B060100000000,
    0x0506060100000000,
    0x020C060100000000,
    0, /* EOL */
};
#define NAME_THROTTLE_SEQUENCE_3 "throttle 3"
static sequence_array_t sequence_throttle_3[SEQUENCE_MAX_STEPS_THROTTLE] = {
    0x0506060100000000,
    0x030B060100000000,
    0x020C060100000000,
    0, /* EOL */
};
#define NAME_THROTTLE_SEQUENCE_4 "throttle 4"
static sequence_array_t sequence_throttle_4[SEQUENCE_MAX_STEPS_THROTTLE] = {
    0x030B060100000000,
    0x0506060100000000,
    0x020C060100000000,
    0, /* EOL */
};
#define NAME_THROTTLE_SEQUENCE_5 "throttle 5"
static sequence_array_t sequence_throttle_5[SEQUENCE_MAX_STEPS_THROTTLE] = {
    0x0506060100000000,
    0x020B060100000000,
    0x030B060100000000,
    0x020C060100000000,
    0, /* EOL */
};
#define NAME_THROTTLE_SEQUENCE_6 "throttle 6"
static sequence_array_t sequence_throttle_6[SEQUENCE_MAX_STEPS_THROTTLE] = {
    0x030B060100000000,
    0x0506060100000000,
    0x020B060100000000,
    0x020C060100000000,
    0, /* EOL */
};

sequence_throttle_t sequence_throttle_sequences[THROTTLE_SEQ_COUNT] = {
    {sequence_throttle_1, NAME_THROTTLE_SEQUENCE_1},
    {sequence_throttle_2, NAME_THROTTLE_SEQUENCE_2},
    {sequence_throttle_3, NAME_THROTTLE_SEQUENCE_3},
    {sequence_throttle_4, NAME_THROTTLE_SEQUENCE_4},
    {sequence_throttle_5, NAME_THROTTLE_SEQUENCE_5},
    {sequence_throttle_6, NAME_THROTTLE_SEQUENCE_6},
};
#endif //THRUSTER_CONTROL_HALO12TEST_THRUSTER_CONTROL_MODULES_SEQUENCE_THROTTLE_CONTROL_THROTTLE_SILVER_HALO_H_
