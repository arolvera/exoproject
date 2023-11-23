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

/******************************************************************************
* INCLUDES
******************************************************************************/

#include "app_emcy.h"

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

/* allocate memory for the emergency code mapping table */
static CO_EMCY_TBL EmcyCode[CO_EMCY_N] = {
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //BENIGN_RESET_OR_NO_ERROR,                // = 0x0
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //HKM_OVER_CURRENT,                        // = 0x1
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //HKM_OVER_VOLTAGE,                        // = 0x2
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //HKM_UNDER_VOLTAGE,                       // = 0x3
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //HKM_SHORT_DETECTED,                      // = 0x4
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //HKM_COMM_LOST,                           // = 0x5
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_1,                              // = 0x6
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_2,                              // = 0x7
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_3,                              // = 0x8
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_4,                              // = 0x9
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_5,                              // = 0xA
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_6,                              // = 0xB
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //ECP_COMM_TIMING_ERROR,                   // = 0xC
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //ECP_COMM_CRC_ERROR,                      // = 0xD
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_7,                              // = 0xE
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //ECP_WATCHDOG_TIMEOUT,                    // = 0xF
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //ECP_MEMORY_FAULT,                        // = 0x10
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //ECP_BROWNOUT_DETECTED,                   // = 0x11
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //ECP_INSTALL_ERROR,                       // = 0x12
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //BL_INSTALL_ERROR,                        // = 0x13
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //ECP_UNKNOWN_RESET,                       // = 0x14
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //ECP_BACKUP_RESET,                        // = 0x15
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //ECP_NRST,                                // = 0x16
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_HW_ERR},                     //ECP_EEFC_EEC_ERROR,                      // = 0x17
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_9,                              // = 0x18
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //MAGI_COMM_UNKNOWN_COMMAND_ERROR,         // = 0x19
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //MAGI_COMM_TIMING_ERROR,                  // = 0x1A
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //MAGI_COMM_CRC_ERROR,                     // = 0x1B
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //MAGI_MAGNET_VOLTAGE_ERROR,               // = 0x1C
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //MAGI_WATCHDOG_TIMEOUT,                   // = 0x1D
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //MAGI_MEMORY_FAULT,                       // = 0x1E
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //MAGI_BROWNOUT_DETECTED,                  // = 0x1F
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //MAGI_MAGNET_UNDER_CURRENT,               // = 0x20
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //MAGI_MAGNET_OVER_CURRENT,                // = 0x21
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_10,                             // = 0x22
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_11,                             // = 0x23
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_12,                             // = 0x24
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_13,                             // = 0x25
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_14,                             // = 0x26
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //MAGO_COMM_UNKNOWN_COMMAND_ERROR,         // = 0x27
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //MAGO_COMM_TIMING_ERROR,                  // = 0x28
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //MAGO_COMM_CRC_ERROR,                     // = 0x29
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //MAGO_MAGNET_VOLTAGE_ERROR,               // = 0x2A
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //MAGO_WATCHDOG_TIMEOUT,                   // = 0x2B
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //MAGO_MEMORY_FAULT,                       // = 0x2C
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //MAGO_BROWNOUT_DETECTED,                  // = 0x2D
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //MAGO_MAGNET_UNDER_CURRENT,               // = 0x2E
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //MAGO_MAGNET_OVER_CURRENT,                // = 0x2F
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_15,                             // = 0x30
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_16,                             // = 0x31
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_17,                             // = 0x32
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_18,                             // = 0x33
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_19,                             // = 0x34
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //ACK_OVER_CURRENT,                        // = 0x35
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //ACK_OVER_VOLTAGE,                        // = 0x36
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //ACK_UNDER_VOLTAGE,                       // = 0x37
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //ACK_SHORT_DETECTED,                      // = 0x38
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_20,                             // = 0x39
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_21,                             // = 0x3A
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_22,                             // = 0x3B
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //ANODE_GENERAL,                           // = 0x3C
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //ANODE_COMM_LOST,                         // = 0x3D
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //ANODE_COMM_UNKNOWN_COMMAND_ERROR,        // = 0x3E
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //ANODE_COMM_TIMING_ERROR,                 // = 0x3F
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //ANODE_COMM_CRC_ERROR,                    // = 0x40
{.Reg = CO_EMCY_REG_TEMP,               .Code = CO_EMCY_CODE_TEMP_ERR},                   //ANODE_HEAT_SINK_TEMP,                    // = 0x41
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //ANODE_WATCHDOG_TIMEOUT,                  // = 0x42
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //ANODE_MEMORY_FAULT,                      // = 0x43
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //ANODE_BROWNOUT_DETECTED,                 // = 0x44
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //ANODE_UNDER_VOLTAGE,                     // = 0x45
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //ANODE_OVER_VOLTAGE,                      // = 0x46
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //ANODE_X_OVER_CURRENT,                    // = 0x47
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //ANODE_Y_OVER_CURRENT,                    // = 0x48
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //ANODE_UNDER_CURRENT,                     // = 0x49
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_GEN_ERR},                    //ANODE_NO_SPARK,                          // = 0x4A
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //ANODE_MSG_OUTPUT_VOLTAGE_BALLANCE,       // = 0x4B
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_24,                             // = 0x4C
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //KEEPER_COMM_LOST,                        // = 0x4D
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //KEEPER_COMM_UNKNOWN_COMMAND_ERROR,       // = 0x4E
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //KEEPER_COMM_TIMING_ERROR,                // = 0x4F
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //KEEPER_COMM_CRC_ERROR,                   // = 0x50
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_GEN_ERR},                    //KEEPER_NO_SPARK_ERROR,                   // = 0x51
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //KEEPER_WATCHDOG_TIMEOUT,                 // = 0x52
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //KEEPER_MEMORY_FAULT,                     // = 0x53
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //KEEPER_BROWNOUT_DETECTED,                // = 0x54
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //KEEPER_OVER_VOLTAGE_VC,                  // = 0x55
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //KEEPER_OVER_VOLTAGE_IC,                  // = 0x56
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //KEEPER_OVER_VOLTAGE,                     // = 0x57
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //KEEPER_CURRENT_LOW,                      // = 0x58
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //ERROR_CODE_KEEPER_WARN_VOLTAGE,          // = 0x59
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_28,                             // = 0x5A
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_29,                             // = 0x5B
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_30,                             // = 0x5C
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //VCC_OVER_CURRENT,                        // = 0x5D
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //VCC_OVER_VOLTAGE,                        // = 0x5E
{.Reg = CO_EMCY_REG_VOLTAGE,            .Code = CO_EMCY_CODE_VOL_ERR},                    //VCC_UNDER_VOLTAGE,                       // = 0x5F
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //VCC_SHORT_DETECTED,                      // = 0x60
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //VCC_COMM_LOST,                           // = 0x61
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_31,                             // = 0x62
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_32,                             // = 0x63
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_33,                             // = 0x64
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_34,                             // = 0x65
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_35,                             // = 0x66
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //VALVE_COMM_UNKNOWN_COMMAND_ERROR,        // = 0x67
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //VALVE_COMM_TIMING_ERROR,                 // = 0x68
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //VALVE_COMM_CRC_ERROR,                    // = 0x69
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_36,                             // = 0x6A
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //VALVE_WATCHDOG_TIMEOUT,                  // = 0x6B
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //VALVE_MEMORY_FAULT,                      // = 0x6C
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_HW_ERR},                     //VALVE_BROWNOUT_DETECTED,                 // = 0x6D
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_37,                             // = 0x6E
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_38,                             // = 0x6F
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_39,                             // = 0x70
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_40,                             // = 0x71
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_41,                             // = 0x72
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_42,                             // = 0x73
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_43,                             // = 0x74
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //MCU_BOOT_ERR,                            // = 0x75
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //MCU_THRUSTER_FAULT,                      // = 0x76
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_44,                             // = 0x77
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_45,                             // = 0x78
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_46,                             // = 0x79
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //THRUSTER_POWER_HIGH_FAULT,               // = 0x7A
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //THRUSTER_POWER_HIGH_WARN,                // = 0x7B
{.Reg = CO_EMCY_REG_CURRENT,            .Code = CO_EMCY_CODE_CUR_ERR},                    //THRUSTER_POWER_LOW_WARN,                 // = 0x7C
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_47,                             // = 0x7D
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_48,                             // = 0x7E
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_49,                             // = 0x7F
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_50,                             // = 0x80
{.Reg = CO_EMCY_REG_MANUFACTURER,       .Code = CO_EMCY_CODE_SW_ERR},                     //MEMORY_SCRUBBER_FAULT,                   // = 0x81
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_51,                             // = 0x82
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_52,                             // = 0x83
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_53,                             // = 0x84
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_54,                             // = 0x85
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_55,                             // = 0x86
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_56,                             // = 0x87
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_57,                             // = 0x88
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_OVERRUN_ERR},        //COMM_OVERRUN_ERROR,                      // = 0x89
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //COMM_CRC_ERROR,                          // = 0x8A
{.Reg = CO_EMCY_REG_COM,                .Code = CO_EMCY_CODE_MON_COM_ERR},                //COMM_FRAME_ERROR,                        // = 0x8B
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_58,                             // = 0x8C
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_59,                             // = 0x8D
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_60,                             // = 0x8E
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_61,                             // = 0x8F
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_62,                             // = 0x90
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_63,                             // = 0x91
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_64,                             // = 0x92
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_65,                             // = 0x93
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_66,                             // = 0x94
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_67,                             // = 0x95
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_68,                             // = 0x96
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //SQNC_READY_MODE_FAULT,                   // = 0x97
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //SQNC_STEADY_STATE_FAULT,                 // = 0x98
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //SQNC_THROTTLING_FAULT,                   // = 0x99
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //SQNC_CONDITIONING_FAULT,                 // = 0x9A
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //SQNC_BIT_FAULT,                          // = 0x9B
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_69,                             // = 0x9C
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_70,                             // = 0x9D
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_71,                             // = 0x9E
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_NO_ERR},                     //RESERVED_72,                             // = 0x9F
{.Reg = CO_EMCY_REG_GENERAL,            .Code = CO_EMCY_CODE_GEN_ERR},                    //UNKNOWN_ERR,                             // = 0xA0 
};

/*---------------------------------------------------------------------------*/
/*! \brief REQ-TEM-0110
*
* \details Return the pointer to the first entry of the emergency map table.
*/
/*---------------------------------------------------------------------------*/
CO_EMCY_TBL *EmcyGetTable(void)
{
    CO_EMCY_TBL *result = &EmcyCode[0];
    return (result);
}
