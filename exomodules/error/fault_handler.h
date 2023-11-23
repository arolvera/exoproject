/**
 * @file    error_handler.h
 *
 * @brief   ??? Summary of this module. What is the diff with error_handler?
 *
 * Is the implementation complete? Any more work to do?
 * Several of the functions are not implemented or unused.
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

#ifndef FAULT_HANDLER_H
#define FAULT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* The TC_EMCY values track the macro values in co_emcy.h */
#define TC_EMCY_REG_GENERAL      0 //CO_EMCY_REG_GENERAL
#define TC_EMCY_REG_CURRENT      1 //CO_EMCY_REG_CURRENT
#define TC_EMCY_REG_VOLTAGE      2 //CO_EMCY_REG_VOLTAGE
#define TC_EMCY_REG_TEMP         3 //CO_EMCY_REG_TEMP
#define TC_EMCY_REG_COM          4 //CO_EMCY_REG_COM

/* Undefine this so no one can use it until we want to */
#define TC_EMCY_REG_PROFILE      5 //CO_EMCY_REG_PROFILE
#undef TC_EMCY_REG_PROFILE

#define TC_EMCY_REG_MANUFACTURER 7 //CO_EMCY_REG_MANUFACTURER
#define TC_EMCY_REG_NUM          8 // CO_EMCY_REG_NUM

typedef enum{
    FH_NO_ACTION                = 0,
    FH_ALERT                    = 1,
    FH_ALERT_PREOP              = 2,
    FH_ALERT_LOCKOUT            = 3,
    FH_ALERT_THRUSTER_SHUTDOWN  = 4,
    FH_ALERT_REINIT_RESTART_APP = 5,
    FH_EOL
} fault_handlers_t;

typedef enum{
    FH_READ  = 0,
    FH_WRITE = 1,
} fh_action_t;

typedef struct fh_info{
    uint8_t e_type; 
    uint8_t e_code;
    uint8_t error_count;
    uint8_t module_num;
    uint8_t submodule_num; 
    uint16_t line_num; 
    bool condition;
} fh_info_t;

int fh_fault_handle(fh_info_t* fh_info);
void fh_fault_handler_init(void);
int fh_fault_config(uint8_t e_code, uint8_t new_fault_handler, fh_action_t action);
int fh_fault_handlers_register(uint8_t fault_num, uint8_t fault_handler);
void* fh_fault_config_get(unsigned int* size);
int fh_fault_handlers_registry_check(uint8_t fault_num);



#ifdef __cplusplus
}
#endif

#endif  //FAULT_HANDLER_H
