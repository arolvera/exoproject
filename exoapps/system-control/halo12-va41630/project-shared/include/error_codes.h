/* 
 * File:   error_codes.h
 * Author: bwelker
 *
 * Created on January 20, 2022, 10:00 AM
 */

#include "ext_decl_define.h"

#ifndef ERROR_CODES_H
#define ERROR_CODES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ERROR_CODE_BENIGN_RESET_OR_NO_ERROR,         // = 0x0
    ERROR_CODE_HKM_OVER_CURRENT,                 // = 0x1
    ERROR_CODE_HKM_OVER_VOLTAGE,                 // = 0x2
    ERROR_CODE_HKM_UNDER_VOLTAGE,                // = 0x3
    ERROR_CODE_HKM_SHORT_DETECTED,               // = 0x4
    ERROR_CODE_HKM_COMM_LOST,                    // = 0x5
    ERROR_CODE_RESERVED_1,                       // = 0x6
    ERROR_CODE_RESERVED_2,                       // = 0x7
    ERROR_CODE_RESERVED_3,                       // = 0x8
    ERROR_CODE_RESERVED_4,                       // = 0x9
    ERROR_CODE_RESERVED_5,                       // = 0xA
    ERROR_CODE_RESERVED_6,                       // = 0xB
    ERROR_CODE_ECP_COMM_TIMING_ERROR,            // = 0xC
    ERROR_CODE_ECP_COMM_CRC_ERROR,               // = 0xD
    ERROR_CODE_RESERVED_7,                       // = 0xE
    ERROR_CODE_ECP_WATCHDOG_TIMEOUT,             // = 0xF
    ERROR_CODE_ECP_MEMORY_FAULT,                 // = 0x10
    ERROR_CODE_ECP_BROWNOUT_DETECTED,            // = 0x11
    ERROR_CODE_ECP_INSTALL_ERROR,                // = 0x12
    ERROR_CODE_BL_INSTALL_ERROR,                 // = 0x13
    ERROR_CODE_ECP_UNKNOWN_RESET,                // = 0x14
    ERROR_CODE_ECP_BACKUP_RESET,                 // = 0x15
    ERROR_CODE_ECP_NRST,                         // = 0x16
    ERROR_CODE_ECP_EEFC_EEC_ERROR,               // = 0x17
    ERROR_CODE_RESERVED_9,                       // = 0x18
    ERROR_CODE_MAGI_COMM_UNKNOWN_COMMAND_ERROR,  // = 0x19
    ERROR_CODE_MAGI_COMM_TIMING_ERROR,           // = 0x1A
    ERROR_CODE_MAGI_COMM_CRC_ERROR,              // = 0x1B
    ERROR_CODE_MAGI_MAGNET_VOLTAGE_ERROR,        // = 0x1C
    ERROR_CODE_MAGI_WATCHDOG_TIMEOUT,            // = 0x1D
    ERROR_CODE_MAGI_MEMORY_FAULT,                // = 0x1E
    ERROR_CODE_MAGI_BROWNOUT_DETECTED,           // = 0x1F
    ERROR_CODE_MAGI_MAGNET_UNDER_CURRENT,        // = 0x20
    ERROR_CODE_MAGI_MAGNET_OVER_CURRENT,         // = 0x21
    ERROR_CODE_RESERVED_10,                      // = 0x22
    ERROR_CODE_RESERVED_11,                      // = 0x23
    ERROR_CODE_RESERVED_12,                      // = 0x24
    ERROR_CODE_RESERVED_13,                      // = 0x25
    ERROR_CODE_RESERVED_14,                      // = 0x26
    ERROR_CODE_MAGO_COMM_UNKNOWN_COMMAND_ERROR,  // = 0x27
    ERROR_CODE_MAGO_COMM_TIMING_ERROR,           // = 0x28
    ERROR_CODE_MAGO_COMM_CRC_ERROR,              // = 0x29
    ERROR_CODE_MAGO_MAGNET_VOLTAGE_ERROR,        // = 0x2A
    ERROR_CODE_MAGO_WATCHDOG_TIMEOUT,            // = 0x2B
    ERROR_CODE_MAGO_MEMORY_FAULT,                // = 0x2C
    ERROR_CODE_MAGO_BROWNOUT_DETECTED,           // = 0x2D
    ERROR_CODE_MAGO_MAGNET_UNDER_CURRENT,        // = 0x2E
    ERROR_CODE_MAGO_MAGNET_OVER_CURRENT,         // = 0x2F
    ERROR_CODE_RESERVED_15,                      // = 0x30
    ERROR_CODE_RESERVED_16,                      // = 0x31
    ERROR_CODE_RESERVED_17,                      // = 0x32
    ERROR_CODE_RESERVED_18,                      // = 0x33
    ERROR_CODE_RESERVED_19,                      // = 0x34
    ERROR_CODE_ACK_OVER_CURRENT,                 // = 0x35
    ERROR_CODE_ACK_OVER_VOLTAGE,                 // = 0x36
    ERROR_CODE_ACK_UNDER_VOLTAGE,                // = 0x37
    ERROR_CODE_ACK_SHORT_DETECTED,               // = 0x38
    ERROR_CODE_RESERVED_20,                      // = 0x39
    ERROR_CODE_RESERVED_21,                      // = 0x3A
    ERROR_CODE_RESERVED_22,                      // = 0x3B
    ERROR_CODE_ANODE_GENERAL,                    // = 0x3C
    ERROR_CODE_ANODE_COMM_LOST,                  // = 0x3D
    ERROR_CODE_ANODE_COMM_UNKNOWN_COMMAND_ERROR, // = 0x3E
    ERROR_CODE_ANODE_COMM_TIMING_ERROR,          // = 0x3F
    ERROR_CODE_ANODE_COMM_CRC_ERROR,             // = 0x40
    ERROR_CODE_ANODE_HEAT_SINK_TEMP,             // = 0x41
    ERROR_CODE_ANODE_WATCHDOG_TIMEOUT,           // = 0x42
    ERROR_CODE_ANODE_MEMORY_FAULT,               // = 0x43
    ERROR_CODE_ANODE_BROWNOUT_DETECTED,          // = 0x44
    ERROR_CODE_ANODE_UNDER_VOLTAGE,              // = 0x45
    ERROR_CODE_ANODE_OVER_VOLTAGE,               // = 0x46
    ERROR_CODE_ANODE_X_OVER_CURRENT,             // = 0x47
    ERROR_CODE_ANODE_Y_OVER_CURRENT,             // = 0x48
    ERROR_CODE_ANODE_UNDER_CURRENT,              // = 0x49
    ERROR_CODE_ANODE_NO_SPARK,                   // = 0x4A
    ERROR_CODE_ANODE_VOLTAGE_BALANCE,            // = 0x4B
    ERROR_CODE_ANODE_OVER_CURRENT,               // = 0x4C
    ERROR_CODE_KEEPER_COMM_LOST,                 // = 0x4D
    ERROR_CODE_KEEPER_COMM_UNKNOWN_COMMAND_ERROR,// = 0x4E
    ERROR_CODE_KEEPER_COMM_TIMING_ERROR,         // = 0x4F
    ERROR_CODE_KEEPER_COMM_CRC_ERROR,            // = 0x50
    ERROR_CODE_KEEPER_NO_SPARK_ERROR,            // = 0x51
    ERROR_CODE_KEEPER_WATCHDOG_TIMEOUT,          // = 0x52
    ERROR_CODE_KEEPER_MEMORY_FAULT,              // = 0x53
    ERROR_CODE_KEEPER_BROWNOUT_DETECTED,         // = 0x54
    ERROR_CODE_KEEPER_OVER_VOLTAGE_STARTUP,      // = 0x55
    ERROR_CODE_KEEPER_OVER_VOLTAGE_CONTROL,      // = 0x56
    ERROR_CODE_KEEPER_OVER_VOLTAGE,              // = 0x57
    ERROR_CODE_KEEPER_CURRENT_LOW,               // = 0x58
    ERROR_CODE_KEEPER_WARN_VOLTAGE,              // = 0x59
    ERROR_CODE_KEEPER_OVER_CURRENT,              // = 0x5A
    ERROR_CODE_RESERVED_29,                      // = 0x5B
    ERROR_CODE_RESERVED_30,                      // = 0x5C
    ERROR_CODE_VCC_OVER_CURRENT,                 // = 0x5D
    ERROR_CODE_VCC_OVER_VOLTAGE,                 // = 0x5E
    ERROR_CODE_VCC_UNDER_VOLTAGE,                // = 0x5F
    ERROR_CODE_VCC_SHORT_DETECTED,               // = 0x60
    ERROR_CODE_VCC_COMM_LOST,                    // = 0x61
    ERROR_CODE_RESERVED_31,                      // = 0x62
    ERROR_CODE_RESERVED_32,                      // = 0x63
    ERROR_CODE_RESERVED_33,                      // = 0x64
    ERROR_CODE_RESERVED_34,                      // = 0x65
    ERROR_CODE_RESERVED_35,                      // = 0x66
    ERROR_CODE_VALVE_COMM_UNKNOWN_COMMAND_ERROR, // = 0x67
    ERROR_CODE_VALVE_COMM_TIMING_ERROR,          // = 0x68
    ERROR_CODE_VALVE_COMM_CRC_ERROR,             // = 0x69
    ERROR_CODE_RESERVED_36,                      // = 0x6A
    ERROR_CODE_VALVE_WATCHDOG_TIMEOUT,           // = 0x6B
    ERROR_CODE_VALVE_MEMORY_FAULT,               // = 0x6C
    ERROR_CODE_VALVE_BROWNOUT_DETECTED,          // = 0x6D
    ERROR_CODE_RESERVED_37,                      // = 0x6E
    ERROR_CODE_RESERVED_38,                      // = 0x6F
    ERROR_CODE_RESERVED_39,                      // = 0x70
    ERROR_CODE_RESERVED_40,                      // = 0x71
    ERROR_CODE_RESERVED_41,                      // = 0x72
    ERROR_CODE_RESERVED_42,                      // = 0x73
    ERROR_CODE_RESERVED_43,                      // = 0x74
    ERROR_CODE_MCU_BOOT_ERR,                     // = 0x75
    ERROR_CODE_MCU_THRUSTER_FAULT,               // = 0x76
    ERROR_CODE_RESERVED_44,                      // = 0x77
    ERROR_CODE_RESERVED_45,                      // = 0x78
    ERROR_CODE_RESERVED_46,                      // = 0x79
    ERROR_CODE_THRUSTER_POWER_HIGH_FAULT,        // = 0x7A
    ERROR_CODE_THRUSTER_POWER_HIGH_WARN,         // = 0x7B
    ERROR_CODE_THRUSTER_POWER_LOW_WARN,          // = 0x7C
    ERROR_CODE_RESERVED_47,                      // = 0x7D
    ERROR_CODE_RESERVED_48,                      // = 0x7E
    ERROR_CODE_RESERVED_49,                      // = 0x7F
    ERROR_CODE_RESERVED_50,                      // = 0x80
    ERROR_CODE_MEMORY_SCRUBBER_FAULT,            // = 0x81
    ERROR_CODE_RESERVED_51,                      // = 0x82
    ERROR_CODE_RESERVED_52,                      // = 0x83
    ERROR_CODE_RESERVED_53,                      // = 0x84
    ERROR_CODE_RESERVED_54,                      // = 0x85
    ERROR_CODE_RESERVED_55,                      // = 0x86
    ERROR_CODE_RESERVED_56,                      // = 0x87
    ERROR_CODE_RESERVED_57,                      // = 0x88
    ERROR_CODE_COMM_OVERRUN_ERROR,               // = 0x89
    ERROR_CODE_COMM_CRC_ERROR,                   // = 0x8A
    ERROR_CODE_COMM_FRAME_ERROR,                 // = 0x8B
    ERROR_CODE_RESERVED_58,                      // = 0x8C
    ERROR_CODE_RESERVED_59,                      // = 0x8D
    ERROR_CODE_RESERVED_60,                      // = 0x8E
    ERROR_CODE_RESERVED_61,                      // = 0x8F
    ERROR_CODE_RESERVED_62,                      // = 0x90
    ERROR_CODE_RESERVED_63,                      // = 0x91
    ERROR_CODE_RESERVED_64,                      // = 0x92
    ERROR_CODE_RESERVED_65,                      // = 0x93
    ERROR_CODE_RESERVED_66,                      // = 0x94
    ERROR_CODE_RESERVED_67,                      // = 0x95
    ERROR_CODE_RESERVED_68,                      // = 0x96
    ERROR_CODE_SQNC_READY_MODE_FAULT,            // = 0x97
    ERROR_CODE_SQNC_STEADY_STATE_FAULT,          // = 0x98
    ERROR_CODE_SQNC_THROTTLING_FAULT,            // = 0x99
    ERROR_CODE_SQNC_CONDITIONING_FAULT,          // = 0x9A
    ERROR_CODE_SQNC_BIT_FAULT,                   // = 0x9B
    ERROR_CODE_RESERVED_69,                      // = 0x9C
    ERROR_CODE_RESERVED_70,                      // = 0x9D
    ERROR_CODE_RESERVED_71,                      // = 0x9E
    ERROR_CODE_RESERVED_72,                      // = 0x9F
    ERROR_CODE_UNKNOWN_ERR,                      // = 0xA0
    ERROR_CODE_EOL

} error_codes_t;

#ifdef __cplusplus
}
#endif

#endif /* ERROR_CODES_H */
