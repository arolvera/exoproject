/**
 * @file    thruster_control.h
 *
 * @brief   Data structures for thruster control.
 *
 * @copyright   Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
 *
 * Unauthorized copying of this file, via any medium is strictly prohibited
 * proprietary and confidential.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior written
 * consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
 * software without notice.
 */

#ifndef THRUSTER_CONTROL_H
#define THRUSTER_CONTROL_H

#include <stdint.h>
#include "client-control/client_booted.h"
#include "ext_decl_define.h"


typedef enum{
  MODULE_NUM_CLIENT_CONTROL,
  MODULE_NUM_SERIAL,
  MODULE_NUM_ICM,
  MODULE_NUM_APP,
  MODULE_NUM_UPDATE_COMMAND,
} module_numbers_t;

/******************************************************************************/
/***************** Hardware Specific IDs and Configs **************************/
/******************************************************************************/
EXT_DECL uint16_t Hardware_ID
#if InitVar
= 0
#endif
;

/* Is the serial interface full duplex (CAN is full duplex) */
EXT_DECL uint8_t Full_Duplex
#if InitVar
= 1 /* Assume it is by default */
#endif
;

typedef enum __task_wd_flags
{
  TASK_FLAG_RESET     = 0x00,
  TASK_FLAG_MAIN      = 0x01,
  TASK_FLAG_CAN_OPEN  = 0x02,
  TASK_FLAG_CAN_INT   = 0x04,
  TASK_FLAG_ALL       = 0x07,
} TASK_FLAGS_t;

EXT_DECL TASK_FLAGS_t task_flags
#if InitVar
= 0
#endif
;


/******************************************************************************/
/***************** Health & Status Globals ************************************/
/******************************************************************************/
/* Health and status ping tick in millisecs, controlled external to this module */
#define DEFAULT_HSI_INTERVAL 250
EXT_DECL  uint32_t Health_Tick_Millisecs
#if InitVar
= DEFAULT_HSI_INTERVAL
#endif
;

/* Health tick are going to get annoy - this turns them off
 * Note - setting them to zero will not turn them off, quite the opposite!
 */
EXT_DECL uint8_t Health_Tick_Enabled
#if InitVar
= 0
#endif
;

/******************************************************************************/

/******************************************************************************/
/***************** Client & Thruster State Globals  ***************************/
/******************************************************************************/


/* This global info tracks the NMT state for use throughout the application for
 * the purpose of tracking state without tieing the whole application to the
 * CAN Open Stack
 */

/* This is mirrors CAN Open Stack and add our states to it
 * !!!! DO NOT CHANGE ANYTHING BEFORE & INCLUDING TC_CO_MODE_NUM !!!!
 */
typedef enum TCS_CO_MODE_T {            /*!< Thruster Control State (TCS)               */
  TCS_CO_INVALID              = 0x0,  /*!< device in INVALID mode                     */
  TCS_CO_INIT                 = 0x1,  /*!< device in INIT mode                        */
  TCS_CO_PREOP                = 0x2,  /*!< device in PRE-OPERATIONAL mode             */
  TCS_CO_OPERATIONAL          = 0x3,  /*!< device in OPERATIONAL mode                 */
  TCS_CO_STOP                 = 0x4,  /*!< device in STOP mode                        */
  TCS_CO_MODE_NUM             = 0x5,  /*!< number of CAN OPEN device modes            */
  TCS_POWER_OFF               = 0x6,  /*!< Powered OFF                                */
  TCS_TRANISTION_STANDBY      = 0x7,  /*!< Transitioning to Standby (NMT_OPERATIONAL) */
  TCS_STANDBY                 = 0x8,  /*!< In Standby               (NMT_OPERATIONAL) */
  TCS_TRANSITION_READY_MODE   = 0x9,  /*!< 'Thruster Startup'       (NMT_OPERATIONAL) */
  TCS_READY_MODE              = 0xA,  /*!< Ready Mode (Keeper On)   (NMT_OPERATIONAL) */
  TCS_TRANSITION_STEADY_STATE = 0xB,  /*!< Starting Anode           (NMT_OPERATIONAL) */
  TCS_STEADY_STATE            = 0xC,  /*!< Steady State (Anode On)  (NMT_OPERATIONAL) */
  TCS_CONDITIONING            = 0xD,  /*!< Steady State (Anode On)  (NMT_OPERATIONAL) */
  TCS_BIT_TEST                = 0xE,  /*!< Steady State (Anode On)  (NMT_OPERATIONAL) */
  TCS_LOCKOUT                 = 0xF,  /*!< Locked after error       (NMT_OPERATIONAL) */
  TCS_STATE_NUM               = 0x10  /*!< Total number of Thruster Control States    */
} TCS_CO_MODE;

EXT_DECL uint32_t Thruster_NMT_State
#if InitVar
= 0 /* Invalid state */
#endif
;

EXT_DECL uint32_t Thruster_State
#if InitVar
= TCS_CO_INVALID;
#endif
;

/* Thruster State from the outside world view point macro
 *
 * IF the we are in the Operational NMT state, then or system state is driven
 * by the Thruster_State variable.  Else it is driven by the NMT state
 */
#define Thruster_Control_State (                    \
    (Thruster_NMT_State == TCS_CO_OPERATIONAL) ?    \
     Thruster_State : Thruster_NMT_State            \
)

/* The Thruster can be in STEADY STATE with the Keeper ON, in which case
 * the thruster state will be 0xAC.  But 'In Steady State' does not care if
 * the keeper is on or off.   Just look at the lower nibble
 */
#define THRUSTER_IN_STEADY_STATE() ((Thruster_State & 0xF) == TCS_STEADY_STATE)
/**
 * Heartbeat milliseconds should be a compile time option, else set it here
 */
#ifndef _HEARTHBEAT_ENABLE
#define HEARTBEAT_MILLISECONDS 0
#else
#define HEARTBEAT_MILLISECONDS _HEARTHBEAT_ENABLE
#endif
EXT_DECL uint16_t Heartbeat_Milliseconds
#if InitVar
= HEARTBEAT_MILLISECONDS
#endif
;

/* Private type, do not use */
typedef struct app_stat_
{
    uint32_t magic_number;
    uint32_t crash_status;
    uint32_t active_bckp_region;
} app_stat_t_;

typedef struct app_stat {
    app_stat_t_ app_status;
    uint8_t reserved[0x100 - sizeof(app_stat_t_) - sizeof(uint16_t)];
    uint16_t crc;
} app_stat_t;

#endif //THRUSTER_CONTROL_H
