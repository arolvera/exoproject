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

#ifndef _MCU_INCLUDE_H
#define _MCU_INCLUDE_H

#include <stdint.h>

#define RESPONSE_PARAMETERS_CAN_ID_BASE     0x180
#define COMMAND_PARAMETERS_CAN_ID_BASE      0x200
#define BROADCAST_STATE_CAN_ID_BASE         0x280
#define BROADCAST_VARIABLE_ID_BASE          0x380
#define HEALTH_CAN_ID_BASE                  0x480
#define BL_CRIS_ID_BASE                     0x7F0

/* Macros to spoof out internal CANopen "segmented transfer".
 * I DON'T THINK THIS IS THE RIGHT PLACE FOR THESE */
#define DEVICE_ID 0
#define APP_NODE_ID           0x22u

#define SEG_TFER_SVR_ID       0x600
#define SEG_TFER_CLT_ID       0x580

#define BL_EMCY               0x80

#define CMD_SERVER_SEG_TFER_INIT          0x21
#define CMD_SERVER_SEG_TFER_LAST_SEG_BIT  0x01


#define CMD_CLIENT_SEG_TFER_INIT_ACK    0x60
#define CMD_CLIENT_SEG_TFER_DATA_ACK    0x20
#define CMD_SEG_TFER_TOGGLE_BIT         0x10
/***********************************************************/


#define COMM_ID_KEEPER     0x001
#define COMM_ID_ANODE      0x002

#define COMM_ID_SYSTEM_CONTROL 0x22

#define COMM_ID_MAGNET_O   0x003
#define COMM_ID_MAGNET_I   0x004
#define COMM_ID_VALVE      0x005
#define COMM_ID_BUCK       0x006

#define COMM_ID_MAGNET   COMM_ID_MAGNET_O //Outer is used in one magnet scenario.

#define RESPONSE_PARAMETERS_ID_BASE     0x180
#define COMMAND_PARAMETERS_ID_BASE      0x200
#define BROADCAST_STATE_ID_BASE         0x280
#define BROADCAST_VARIABLE_ID_BASE      0x380
#define HEALTH_ID_BASE                  0x480

#define HEALTH_ENTRIES  3
#define CAN_HSI_SYNC    3
#define HSI_ID              0x080

//Get component commid from message_t id
#define COMPONENT_COMM_ID_MASK (0xF)


#define  BL_CRIS_ID_ACP   BL_CRIS_ID_BASE | COMM_ID_ANODE

#define  BL_CRIS_ID_MVP   BL_CRIS_ID_BASE | COMM_ID_MAGNET_O

#define BOOTLOADER_IMAGE_REQ_ECPK   (BL_CRIS_ID_BASE | COMM_ID_ECPK)
#define BOOTLOADER_IMAGE_REQ_ACP    (BL_CRIS_ID_BASE | COMM_ID_ACP)
#define BOOTLOADER_IMAGE_REQ_MVP    (BL_CRIS_ID_BASE | COMM_ID_MVP)

#define RESPONSE_PARAMETERS_ID_KEEPER (RESPONSE_PARAMETERS_ID_BASE | COMM_ID_KEEPER)
#define COMMAND_PARAMETERS_ID_KEEPER  (COMMAND_PARAMETERS_ID_BASE  | COMM_ID_KEEPER)
#define BROADCAST_STATE_ID_KEEPER     (BROADCAST_STATE_ID_BASE     | COMM_ID_KEEPER)
#define HEALTH_ID_KEEPER              (HEALTH_ID_BASE              | COMM_ID_KEEPER)
#define BROADCAST_VARIABLE_ID_KEEPER  (BROADCAST_VARIABLE_ID_BASE  | COMM_ID_KEEPER)

#define RESPONSE_PARAMETERS_ID_ANODE (RESPONSE_PARAMETERS_ID_BASE | COMM_ID_ANODE)
#define COMMAND_PARAMETERS_ID_ANODE  (COMMAND_PARAMETERS_ID_BASE  | COMM_ID_ANODE)
#define BROADCAST_STATE_ID_ANODE     (BROADCAST_STATE_ID_BASE     | COMM_ID_ANODE)
#define HEALTH_ID_ANODE              (HEALTH_ID_BASE              | COMM_ID_ANODE)
#define BROADCAST_VARIABLE_ID_ANODE  (BROADCAST_VARIABLE_ID_BASE  | COMM_ID_ANODE)

#define RESPONSE_PARAMETERS_ID_MAGNET (RESPONSE_PARAMETERS_ID_BASE | COMM_ID_MAGNET)
#define COMMAND_PARAMETERS_ID_MAGNET  (COMMAND_PARAMETERS_ID_BASE  | COMM_ID_MAGNET)
#define BROADCAST_STATE_ID_MAGNET     (BROADCAST_STATE_ID_BASE     | COMM_ID_MAGNET)
#define HEALTH_ID_MAGNET              (HEALTH_ID_BASE              | COMM_ID_MAGNET)
#define BROADCAST_VARIABLE_ID_MAGNET  (BROADCAST_VARIABLE_ID_BASE  | COMM_ID_MAGNET)

#define RESPONSE_PARAMETERS_ID_VALVE (RESPONSE_PARAMETERS_ID_BASE | COMM_ID_VALVE)
#define COMMAND_PARAMETERS_ID_VALVE  (COMMAND_PARAMETERS_ID_BASE  | COMM_ID_VALVE)
#define BROADCAST_STATE_ID_VALVE     (BROADCAST_STATE_ID_BASE     | COMM_ID_VALVE)
#define HEALTH_ID_VALVE              (HEALTH_ID_BASE              | COMM_ID_VALVE)
#define BROADCAST_VARIABLE_ID_VALVE  (BROADCAST_VARIABLE_ID_BASE  | COMM_ID_VALVE)

#define RESPONSE_PARAMETERS_ID_BUCK (RESPONSE_PARAMETERS_ID_BASE | COMM_ID_BUCK)
#define COMMAND_PARAMETERS_ID_BUCK  (COMMAND_PARAMETERS_ID_BASE  | COMM_ID_BUCK)
#define BROADCAST_STATE_ID_BUCK     (BROADCAST_STATE_ID_BASE     | COMM_ID_BUCK)
#define HEALTH_ID_BUCK              (HEALTH_ID_BASE              | COMM_ID_BUCK)
#define BROADCAST_VARIABLE_ID_BUCK  (BROADCAST_VARIABLE_ID_BASE  | COMM_ID_BUCK)

/* Power on/off.  All supplies and latch valves follow this format */
typedef enum {
    COMMAND_SWITCH_ON  = 0x2a,
    COMMAND_SWITCH_OFF = 0x15,
} COMMAND_SWITCH;

/* An enum for scaling reporting */
typedef enum {
    COUNTS_PER_VOLT_INPUT,
    COUNTS_PER_VOLT_OUTPUT,
    COUNTS_PER_AMPERE_INPUT,        // Gotta use the full unit outta respect for Andre-Marie
    COUNTS_PER_AMPERE_OUTPUT,
    COUNTS_PER_PSI_LOW_PRESSURE,    // For the fifty PSI pressure transducers
    COUNTS_PER_PSI_HIGH_PRESSURE,   // For the three-thousand/five-thousand PSI pressure transducers
    THERMISTOR_BETA,
    THERMISTOR_R_NOUGHT,
    TEMPERATURE_LINEAR
} unit_conversions_t;

typedef enum {
    HEALTH_ENTRY_SIZE_EOL = 0,
    HEALTH_ENTRY_SIZE_1   = 1,
    HEALTH_ENTRY_SIZE_2   = 2,
    HEALTH_ENTRY_SIZE_4   = 4,
} health_entry_size_t;

typedef struct health_table_entry {
    volatile void *data;
    health_entry_size_t  size;
    void* health_error_history;
    uint16_t error_history_num_entries;
    uint16_t error_history_count;
    uint8_t  signed_val;
} health_table_entry_t;

//Health table array
typedef health_table_entry_t *health_array[HEALTH_ENTRIES];

typedef enum {
    VERSION_INFO,
    SCALING_INFO,
    IOUT_ACC_INFO,
} CAN_variable_type_t;
typedef uint8_t variable_type_t;

#pragma GCC diagnostic ignored "-Wpragmas"
#pragma pack(push, 1)
typedef struct{
    variable_type_t type;   // Any variables sent over CAN must have the variable type as the first element
    uint8_t major;
    uint8_t minor;
    uint8_t rev;
    uint32_t git_sha;
} mcu_version_t;

typedef struct{
    variable_type_t type;   // Any variables sent over CAN must have the variable type as the first element
    uint8_t unused;
    uint16_t count;
    uint32_t accumulator;
} accumulator_t;

typedef struct {
    variable_type_t type;   // Any variables sent over CAN must have the variable type as the first element
    uint8_t unit_description;
    uint8_t unused[2];
    float scaling;
} bcast_scaling_t;

typedef struct _mcu_can_house_keeping {
    uint8_t ofb_ins;
    uint8_t ofb_rem;
    uint8_t ofb_head;
    uint8_t ofb_tail;
}mcu_can_house_keeping_t;


/***************** Common Command Structure ***********************************/

/* Function Pointer Typedef */
typedef void (*CMD_t)(uint8_t *data);

typedef enum {
    OFF_SET_POINT = 0x10,
    ON_SET_POINT  = 0x20
} on_off_t;

typedef enum {
    ON_OFF          = 0x10,
    SET_VOLTAGE     = 0x20,
    SET_CURRENT     = 0x30,
    SET_FLOW        = 0x40,
    INVALIDATE_IMG  = 0x62,
    PROCESSOR_RESET = 0xFF
} command_t;

typedef enum {
    NO_SPECIFIER  = 0x00,
    INNER_MAG     = 0x10,
    MAGNET        = 0x20,
    CAT_HIGH_FLOW = 0x30,
    CAT_LOW_FLOW  = 0x40,
    ANODE_FLOW    = 0x50,
    RESTART       = 0x60
} specifier_t;

typedef struct _command_function
{
    command_t command;
    CMD_t CMD;
} command_function_t;

typedef struct _command_table
{
    command_function_t *CMDS;
    uint16_t n_commands;
} command_table_t;

/* Command/Response Structures */
typedef struct command_structure {
    command_t command;
    specifier_t specifier;
    uint16_t set_point;
    uint8_t unused[4];
} command_structure_t;

typedef enum {
    BCAST_INITIAL_BOOT,
    BCAST_STATE_CHANGE,
    BCAST_ERROR = 0xFF,
} bcast_type_t;

// Generic reasons for state change that the system control needs to know about
typedef enum {
    INIT_COMPLETE  = 0x10,
    COMMANDED_ON   = 0x20,
    COMMANDED_OFF  = 0x30,
    POWER_GOOD     = 0x40,  //Mags
    SPARK_DETECTED = 0x50,  //Keeper, anode
    ERROR_DETECTED = 0x60,
    ERROR_CLEARED  = 0x70
} bcast_state_change_reason_t;

/* Generic errors shared by all MCUs. These errors will not cause the MCUs to go
 * into their error states, but just tell the system controller something is 
 * wrong. */
// Start at 0xA0 to avoid overlap with MCU specific errors, which start at 0x00
// 0xA_ for command related errors
// 0xC_ for control related errors
typedef enum {
    // MCU does not recognize that command
    // E.g. commanding the valves to set current
    INVALID_COMMAND   = 0xA0,

    // MCU recognizes the command, but the setpoint is outside the valid range
    // E.g. setting anode voltage to 1kV
    INVALID_SETPOINT  = 0xA1,

    // The command is valid, but the MCU cannot execute it in its current state
    // E.g. commanding the anode on when it is in error state
    INVALID_STATE     = 0xA2,

    // The control signal (DAC, PWM, whatever) has reached its maximum and no
    // more power can be provided
    CONTROL_SATURATED = 0xC0,
} bcast_generic_error_t;

/* Initial boot broadcast */
typedef struct bcast_initial_boot {
    bcast_type_t bcast_type; // always 0x00
    uint8_t boot_status;
    uint8_t unused_1; // 7 byte pad to keep bcast message lined up
    uint8_t boot_id;
    uint8_t unused[4]; // 7 byte pad to keep bcast message lined up
} bcast_initial_boot_t;

/* State change broadcast */
typedef struct bcast_state_change {
    bcast_type_t bcast_type; // always 0x01
    uint8_t state; // new state
    uint8_t reason; // why the state changed
    uint8_t unused[5]; // 5 byte pad to keep bcast message lined up
} bcast_state_change_t;

/* Error broadcast */
typedef struct bcast_error {
    bcast_type_t bcast_type; // always 0xFF
    uint8_t error_code;
    uint16_t adc_val; // Optional ADC value that generated the error
    uint8_t unused[4]; // 4 byte pad to keep bcast message lined up
} bcast_error_t;


/* Command/Response Union - MAX Byte = 8*/
#define MAX_MSG_SIZE 8
typedef union communication_union_t_ {
    uint8_t data[MAX_MSG_SIZE];

    command_structure_t   command;

    bcast_initial_boot_t  bcast_initial_boot;
    bcast_state_change_t  bcast_state_change;
    mcu_version_t         bcast_version;
    bcast_scaling_t       bcast_scaling;
    bcast_error_t         bcast_error;
} communication_union_t;

/***************** End Common Command Structure *******************************/

#pragma pack(pop)
#pragma GCC diagnostic pop

#define DECLARE_CAN_HEALTH_ENTRY(_MSG_CNT_, _CAN_ERR_, _OFB_INS_, _OFB_REM_, _OFB_HED_, _OFB_TAL_) { \
    {.data = _MSG_CNT_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _CAN_ERR_, .size = HEALTH_ENTRY_SIZE_2},   \
    {.data = _OFB_INS_, .size = HEALTH_ENTRY_SIZE_1},   \
    {.data = _OFB_REM_, .size = HEALTH_ENTRY_SIZE_1},   \
    {.data = _OFB_HED_, .size = HEALTH_ENTRY_SIZE_1},   \
    {.data = _OFB_TAL_, .size = HEALTH_ENTRY_SIZE_1},   \
    {.data = 0,         .size = HEALTH_ENTRY_SIZE_EOL}, \
}


#endif
