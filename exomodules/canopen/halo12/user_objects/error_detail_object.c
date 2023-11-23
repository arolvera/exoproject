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

#include "firmware_versions_object.h" // Header for this module
#include "client-control/client_control.h" //For client version pointers
//#include "include/version.h" // For System control version
#include "iacm/iacm.h"
#include "utils/macro_tools.h"
//#include "icm/tc_icm.h"
#include "serial/serial_task.h"
#include "halo12-vorago/canopen/user_object_od_indexes.h"

#define MAX_ERROR_DETAIL_HISTORY     0x10


static uint8_t fault_code_select;

CO_OBJ_DOM error_detail_serial_domain;
#define CO_ERROR_DETAIL_SERIAL_READ  ((uintptr_t)&error_detail_serial_domain)

CO_OBJ_DOM error_detail_client_control_domain;
#define CO_ERROR_DETAIL_CLIENT_CONTROL_READ  ((uintptr_t)&error_detail_client_control_domain)

CO_OBJ_DOM error_detail_app_domain;
#define CO_ERROR_DETAIL_APP_READ     ((uintptr_t)&error_detail_app_domain)

CO_OBJ_DOM error_detail_tc_icm_domain;
#define CO_ERROR_DETAIL_TC_ICM_READ  ((uintptr_t)&error_detail_tc_icm_domain)

CO_OBJ_DOM error_detail_fault_status_domain;
#define CO_ERROR_DETAIL_FAULT_STATUS_READ  ((uintptr_t)&error_detail_fault_status_domain)


CO_OBJ_DOM error_detail_magnet_control_domain;
#define CO_ERROR_DETAIL_MAGNET_CONTROL_READ  ((uintptr_t)&error_detail_magnet_control_domain)

static CO_ERR ErrorDetailWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);

static CO_ERR ErrorDetailRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size);


CO_OBJ_TYPE ErrorDetailType = {
    0,
    0,       
    ErrorDetailRead,
    ErrorDetailWrite,   /* type function to write object content */
};
#define CO_TERRORDETAIL  ((CO_OBJ_TYPE*)&ErrorDetailType)


typedef enum {
    ERR_DET_SUBIDX_COUNT,
    ERR_DET_SERIAL,   
    ERR_DET_CLIENT_CONTROL,
    ERR_DET_TC_ICM, 
    ERR_DET_APP,  
    ERR_DET_EOL
} OD_ERR_DET;

typedef enum{
    FAULT_STATUS_COUNT,
    FAULT_STATUS_DUMP,
    FAULT_STATUS_DATA_WORD_0,
    FAULT_STATUS_DATA_WORD_1,
    FAULT_STATUS_DATA_WORD_2,
    FAULT_STATUS_DATA_WORD_3,
    FAULT_STATUS_DATA_WORD_4,
    FAULT_HANDLER_EOL,
} FAULT_HANDLER;

typedef enum{
    FAULT_CONFIG_COUNT,
    FAULT_CONFIG_FAULT_CODE_SELECT,
    FAULT_CONFIG_REACTION_TYPE,
    FAULT_CONFIG_EOL,
} FAULT_CONFIG;

typedef struct error_settings{
    uint8_t module;
    uint8_t submodule;
    uint8_t errno;
    uint8_t error_count;
}error_settings_t;

typedef struct fault_handler_configs{
    uint8_t error_type;
    uint8_t error_code;
    uint8_t fault_handler_num;
} fault_handler_configs_t;



/*
 * Sets up domain objects for error detail and fault handling to the outside world
 */
void ErrorDetailOD(OD_DYN *self)
{
    /* Number of OD indices */
    ODAdd(self, CO_KEY(OD_INDEX_ERROR_DETAIL, ERR_DET_SUBIDX_COUNT, 
        CO_UNSIGNED8  |CO_OBJ_D__R_), 0, ERR_DET_EOL);
    
    
    int log_size = 0;
    int log_head = 0;
    error_detail_t* e         = eh_get(MODULE_NUM_CLIENT_CONTROL, CLIENT_CONTROL_SUBMODULE);
    ERROR_DETAIL_STRATEGY e_s = eh_module_strategy_get(MODULE_NUM_CLIENT_CONTROL, CLIENT_CONTROL_SUBMODULE, &log_size, &log_head);
    if(e != NULL && e_s != NULL){
        error_detail_client_control_domain.Offset = 0;
        error_detail_client_control_domain.Size   = e->error_detail_log_len * client_error_detail_size_get();
        error_detail_client_control_domain.Start  = e_s(NULL);
            /* Set up domain objects per module/submodule */
        ODAdd(self, CO_KEY(OD_INDEX_ERROR_DETAIL, ERR_DET_CLIENT_CONTROL,
            CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_ERROR_DETAIL_CLIENT_CONTROL_READ);
    } 
#ifndef PRODUCTION_BUILD
    else {
        /* If you reach this point, you may not have registered or init'd your error handling correctly */
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
        __builtin_software_breakpoint();
#endif
        while (true)
        {
        }
    }
#endif
    
    e   = eh_get(MODULE_NUM_APP, APP_SUBMODULE);
    e_s = eh_module_strategy_get(MODULE_NUM_APP, APP_SUBMODULE, &log_size, &log_head);
    if(e != NULL && e_s != NULL){
        error_detail_app_domain.Offset = 0;
        error_detail_app_domain.Size   = e->error_detail_log_len * APP_error_detail_size_get();
        error_detail_app_domain.Start  = e_s(NULL);    
        ODAdd(self, CO_KEY(OD_INDEX_ERROR_DETAIL, ERR_DET_APP,
            CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_ERROR_DETAIL_APP_READ);
    }
#ifndef PRODUCTION_BUILD
    else {
        /* If you reach this point, you may not have registered or init'd your error handling correctly */
#if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
        __builtin_software_breakpoint();
#endif
        while (true)
        {
        }       
    }
#endif
    
    e   = eh_get(MODULE_NUM_SERIAL, COMMS_SERIAL_SUBMODULE);
    e_s = eh_module_strategy_get(MODULE_NUM_SERIAL, COMMS_SERIAL_SUBMODULE, &log_size, &log_head);
    if(e != NULL && e_s != NULL){
        error_detail_serial_domain.Offset = 0;
        error_detail_serial_domain.Size   = e->error_detail_log_len * sizeof(serial_error_detail_t);
        error_detail_serial_domain.Start  = e_s(NULL);
        ODAdd(self, CO_KEY(OD_INDEX_ERROR_DETAIL, ERR_DET_SERIAL,
            CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_ERROR_DETAIL_SERIAL_READ);
    }
#ifndef PRODUCTION_BUILD
    /* If you reach this point, you may not have registered or init'd your error handling correctly */
    else {
 #if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
        __builtin_software_breakpoint();
#endif
        while (true)
        {
        }       
    }
#endif
    
    e   = eh_get(MODULE_NUM_ICM, TC_ICM_SUBMODULE);
    e_s = eh_module_strategy_get(MODULE_NUM_ICM, TC_ICM_SUBMODULE, &log_size, &log_head);
    if(e != NULL && e_s != NULL){
        error_detail_tc_icm_domain.Offset = 0;
        error_detail_tc_icm_domain.Size   = e->error_detail_log_len * tc_icm_error_detail_size_get();
        error_detail_tc_icm_domain.Start  = e_s(NULL);  
        ODAdd(self, CO_KEY(OD_INDEX_ERROR_DETAIL, ERR_DET_TC_ICM,
            CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_ERROR_DETAIL_TC_ICM_READ);
    }
#ifndef PRODUCTION_BUILD
    /* If you reach this point, you may not have registered or init'd your error handling correctly */
    else {
 #if defined(__DEBUG) || defined(__DEBUG_D) && defined(__XC32)
        __builtin_software_breakpoint();
#endif
        while (true)
        {
        }       
    }
#endif  
    
    /* Number of OD indices */
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_COUNT, 
        CO_UNSIGNED8  |CO_OBJ_D__R_), 0, FAULT_HANDLER_EOL);

    fault_status_reg_stat_t fr_stat = {0};
    eh_fault_stat(&fr_stat);
    
    error_detail_fault_status_domain.Offset = 0;
    error_detail_fault_status_domain.Start  = fr_stat.reg_start;
    error_detail_fault_status_domain.Size   = fr_stat.size;

    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_DUMP,
        CO_DOMAIN|CO_OBJ____R_), CO_TDOMAIN, CO_ERROR_DETAIL_FAULT_STATUS_READ);

    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_DATA_WORD_0,
        CO_UNSIGNED32|CO_OBJ____R_), 0, (uintptr_t)&(((uint32_t*)fr_stat.reg_start)[0]));
    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_DATA_WORD_1,
        CO_UNSIGNED32|CO_OBJ____R_), 0, (uintptr_t)&(((uint32_t*)fr_stat.reg_start)[1]));
    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_DATA_WORD_2,
        CO_UNSIGNED32|CO_OBJ____R_), 0,(uintptr_t)&(((uint32_t*)fr_stat.reg_start)[2]));
    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_DATA_WORD_3,
        CO_UNSIGNED32|CO_OBJ____R_), 0, (uintptr_t)&(((uint32_t*)fr_stat.reg_start)[3]));
    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_STATUS, FAULT_STATUS_DATA_WORD_4,
        CO_UNSIGNED32|CO_OBJ____R_), 0, (uintptr_t)&(((uint32_t*)fr_stat.reg_start)[4]));

    

    ODAdd(self, CO_KEY(OD_INDEX_FAULT_REACTION_TYPE, FAULT_CONFIG_COUNT, 
        CO_UNSIGNED8  |CO_OBJ_D__R_), 0, FAULT_CONFIG_EOL);
    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_REACTION_TYPE, FAULT_CONFIG_FAULT_CODE_SELECT,
        CO_UNSIGNED8|CO_OBJ____RW), CO_TERRORDETAIL, (uintptr_t)&ErrorDetailType);
    
    ODAdd(self, CO_KEY(OD_INDEX_FAULT_REACTION_TYPE, FAULT_CONFIG_REACTION_TYPE,
        CO_UNSIGNED8|CO_OBJ____RW), CO_TERRORDETAIL, (uintptr_t)&ErrorDetailType);
    
}


/*
 * Special handling to reconfigure fault handlers from the outside world
 */
static CO_ERR ErrorDetailWrite(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int write_err = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);
    uint8_t fault_reaction_type = 0;

    switch(subidx) {
        case FAULT_CONFIG_FAULT_CODE_SELECT:
            fault_code_select = *(uint8_t*)buf;
            break;
        case FAULT_CONFIG_REACTION_TYPE:
            fault_reaction_type = *(uint8_t*)buf;
            write_err = fh_fault_config(fault_code_select, fault_reaction_type, FH_WRITE);
            break;
        default:
            write_err = __LINE__; // Not implemented
    }
    return write_err ? CO_ERR_TYPE_WR:CO_ERR_NONE;
}


/*
 * Special handling to read fault handler configurations from the outside world
 */
static CO_ERR ErrorDetailRead(struct CO_OBJ_T *obj, struct CO_NODE_T *node, void* buf, uint32_t size)
{
    int read_err = 0;
    int fault_handler_value = 0;
    uint8_t subidx = CO_GET_SUB(obj->Key);

    switch(subidx) {
        case FAULT_CONFIG_FAULT_CODE_SELECT:
            *(uint8_t*)buf = fault_code_select;
            break;
        case FAULT_CONFIG_REACTION_TYPE:
            /* new fault reaction parameter ignored when reading faults */
            fault_handler_value = fh_fault_config(fault_code_select, 0, FH_READ);
            if(fault_handler_value != -1){
                *(int*)buf = fault_handler_value;
            } else{
                /* Invalid fault code or fault value */
                read_err = __LINE__;
            }
            break;
        default:
            read_err = __LINE__; // Not implemented
    }
    return read_err ? CO_ERR_TYPE_RD:CO_ERR_NONE;
}



/* *****************************************************************************
 End of File
 */
