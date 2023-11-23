// Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
//
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//  distribution, or disclosure of this software is expressly forbidden.
//
//  This Copyright notice may not be removed or modified without prior written
//  consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//  ExoTerra Corp
//  7640 S. Alkire Pl.
//  Littleton, CO 80127
//  USA
//
//  Voice:  +1 1 (720) 788-2010
//  http:   www.exoterracorp.com
//  email:  contact@exoterracorp.com
//

#include "control_bit.h"
#define DECLARE_GLOBALS
#include "ext_decl_define.h"

#define NAME_BIT_TEST1 "BIT_TEST1"
static sequence_array_t sequence_bit_test1[SEQUENCE_MAX_STEPS_BIT] = {
    /* BLANK FOR GENERIC UPLOAD */
    0, /* EOL */
};

#define NAME_BIT_LATCH_VALVE_OPEN "BIT_LATCH_VALVE_OPEN"
static sequence_array_t sequence_bit_latch_valve_open[] = {
    0x0504020100000001,
    0, /* EOL */
};

#define NAME_BIT_LATCH_VALVE_CLOSE "BIT_LATCH_VALVE_CLOSE"
static sequence_array_t sequence_bit_latch_valve_close[] = {
    0x0504020100000000,
    0, /* EOL */
};

#define NAME_BIT_CATHODE_LOW_FLOW_CHECK "BIT_CATHODE_LOW_FLOW_CHECK"
static sequence_array_t sequence_bit_cathode_low_flow_check[] = {
    0x0501020100001518,
    0x0001020100003A98,
    0x0501020100002454,
    0x0001020100003A98,
    0, /* EOL */
};

#define NAME_BIT_ANODE_VALVE_CHECK "BIT_ANODE_VALVE_CHECK"
static sequence_array_t sequence_bit_anode_valve_check[] = {
    0x0503020100001DB0,
    0x0001020100003A98,
    0x050302010000399E,
    0x0001020100003A98,
    0, /* EOL */
};

#define NAME_BIT_HF_VALVE_OPEN "BIT_HF_VALVE_OPEN"
static sequence_array_t sequence_bit_hf_valve_open[] = {
    0x05020201000182B8,
    0x00010201001B7740,
    0, /* EOL */
};

#define NAME_COIL_TEST "BIT_COIL_TEST"
static sequence_array_t sequence_coil_test[] = {
    0x03010201000007D0,
    0x0309020100000000,
    0x0302020100000000,
    0x0303020100000000,
    0x00010201000003E8,
    0x0305030100000000,
    0x00010201000007D0,
    0x0308030100000000,
    0x000102010000EA60,
    0, /* EOL */
};

#define NAME_KEEPER_TEST "BIT_KEEPER_TEST"
static sequence_array_t sequence_keeper_test[] = {
#ifdef HALO_SIM
    0x0601020200000001,
#endif
    0x01010809000249F0,
    0x01020809000000FB,
    0x010608090000000A,
    0x0001080900001388,
    0, /* EOL */
};

#define NAME_ANODE_TEST "BIT_ANODE_TEST"
static sequence_array_t sequence_anode_test[] = {
#ifdef HALO_SIM
    0x0602020200000001,
#endif
    0x0201020100030D40,
    0x020202010000000A,
    0x0206020300000005,
    0x0001020100001388,
    0x0204020300000000,
    0, /* EOL */
};

#define NAME_CATH_LF_CHECK_AMBIENT "BIT_CATH_LF_CHECK_AMBIENT"
static sequence_array_t sequence_cathode_lf_check_ambient[] = {
    0x0501020100009C40,
    0x0001020100007530,
    0, /* EOL */
};

#define NAME_ANODE_VALVE_CHECK_AMBIENT "BIT_ANODE_VALVE_CHECK_AMBIENT"
static sequence_array_t sequence_anode_valve_check_ambient[] = {
    0x0503020100009C40,
    0x0001020100007530,
    0, /* EOL */
};

#define NAME_OPEN_ALL_VALVES "BIT_OPEN_ALL_VALVES"
static sequence_array_t sequence_open_all_valves[] = {
    0x0503020100009C40,
    0x0501020100009C40,
    0x05020201000182B8,
    0x00010201001B7740,
    0, /* EOL */
};

//Global var referenced in control_bit
sequence_bit_t bit_seqs[] = {
    {sequence_bit_test1 , NAME_BIT_TEST1,
        sizeof(sequence_bit_test1) / sizeof(sequence_array_t)},
    {sequence_bit_latch_valve_open , NAME_BIT_LATCH_VALVE_OPEN,
        sizeof(sequence_bit_latch_valve_open) / sizeof(sequence_array_t)},
    {sequence_bit_latch_valve_close, NAME_BIT_LATCH_VALVE_CLOSE,
        sizeof(sequence_bit_latch_valve_close) / sizeof(sequence_array_t)},
    {sequence_bit_cathode_low_flow_check, NAME_BIT_CATHODE_LOW_FLOW_CHECK,
        sizeof(sequence_bit_cathode_low_flow_check) / sizeof(sequence_array_t)},
    {sequence_bit_anode_valve_check, NAME_BIT_ANODE_VALVE_CHECK,
        sizeof(sequence_bit_anode_valve_check) / sizeof(sequence_array_t)},
    {sequence_bit_hf_valve_open , NAME_BIT_HF_VALVE_OPEN,
        sizeof(sequence_bit_hf_valve_open) / sizeof(sequence_array_t)},
    {sequence_coil_test , NAME_COIL_TEST,
        sizeof(sequence_coil_test) / sizeof(sequence_array_t)},
    {sequence_keeper_test , NAME_KEEPER_TEST,
        sizeof(sequence_keeper_test) / sizeof(sequence_array_t)},
    {sequence_anode_test , NAME_ANODE_TEST,
        sizeof(sequence_anode_test) / sizeof(sequence_array_t)},
    {sequence_cathode_lf_check_ambient , NAME_CATH_LF_CHECK_AMBIENT,
        sizeof(sequence_cathode_lf_check_ambient) / sizeof(sequence_array_t)},
    {sequence_anode_valve_check_ambient , NAME_ANODE_VALVE_CHECK_AMBIENT,
        sizeof(sequence_anode_valve_check_ambient) / sizeof(sequence_array_t)},
    {sequence_open_all_valves, NAME_OPEN_ALL_VALVES,
        sizeof(sequence_open_all_valves) / sizeof(sequence_array_t)},
};
size_t bit_seq_size = SIZEOF_ARRAY(bit_seqs);