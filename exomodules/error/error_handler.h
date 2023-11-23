/**
 * @file    error_handler.h
 *
 * @brief   ??? Summary of this module. What is the diff with fault_handler?
 *
 * Is the implementation complete? Any more work to do?
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

#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include "error/fault_handler.h"   //for fh_fault_handle()
#include "utils/macro_tools.h" //for SIZEOF_ARRAY())
#include "error_codes.h"



#define CO_ERROR_CLR false
#define CO_ERROR_SET true



typedef enum {
    LOG_LENGTH_2  = 2,
    LOG_LENGTH_4  = 4,
    LOG_LENGTH_8  = 8,
    LOG_LENGTH_16 = 16,
    LOG_LENGTH_32 = 32,
}log_length_t;

#define MAX_ERROR_COUNT 0x10
typedef void* (*ERROR_DETAIL_STRATEGY)(void*);

/* Add error count to base detail */
typedef struct base_error_detail{
    uint32_t e_cnt;
    uint16_t line;
    uint8_t  e_type;
    uint8_t  e_code;
    uint32_t id;
} base_error_detail_t;

typedef struct error_detail{
    uint8_t  module;
    uint8_t  submodule;
    uint16_t error_count;
    uint8_t  log_head;
    uint8_t _pad[3];
    ERROR_DETAIL_STRATEGY error_detail_strategy;
    void* error_log;
    uint32_t error_detail_log_len;
} error_detail_t;


/*
 * fault register status struct
 */
typedef struct{
    uint8_t* reg_start;
    uint32_t size;
}fault_status_reg_stat_t;

typedef struct specific_error_detail{
    base_error_detail_t b_d;
    void* specific_detail;
}specific_error_detail_t;

void eh_clr_trc_msg(int mod, int sub, int e_cnt, int e_type__, int e_code__, int line);
void eh_set_trc_msg(int mod, int sub, int e_cnt, int e_type__, int e_code__, int line);

ERROR_DETAIL_STRATEGY  eh_module_strategy_get(int module, int submodule, int* log_size, int* log_head);

int eh_module_register(error_detail_t* module_error_detail);
error_detail_t* eh_get(uint8_t module, uint8_t submodule);
int eh_create(uint8_t module, uint8_t submodule, ERROR_DETAIL_STRATEGY err_detail_strategy, log_length_t log_size, void* e_log);
int eh_fault_status_set(unsigned int e_type, unsigned int e_code);
int eh_fault_stat(fault_status_reg_stat_t* fr_stat);
int eh_fault_status_clear(unsigned int e_type, unsigned int e_code);
int eh_fault_status_get(unsigned int e_code);
int eh_fault_type_count_get(unsigned int e_type);
void eh_init(void);


#define ERROR_SET__(__e_type__, __e_code__, __detail__)                                                         \
        uint16_t line =  __LINE__;                                                                              \
        error_detail_t* err_set_detail = eh_get(MODULE_NUM, SUBMODULE_NUM);                                     \
        uint32_t e_cnt = err_set_detail->error_count  & (err_set_detail->error_detail_log_len - 1);             \
        specific_error_detail_t b = {.b_d.e_type = (__e_type__), .b_d.e_code = (__e_code__), .b_d.line = line,  \
                                 .b_d.e_cnt = err_set_detail->error_count, .b_d.id = SUBMODULE_NUM,             \
                                 .specific_detail = (__detail__)};                                              \
        err_set_detail->error_detail_strategy((void*)&b);                                                       \
        err_set_detail->module      = MODULE_NUM;                                                               \
        err_set_detail->submodule   = SUBMODULE_NUM;                                                            \
        err_set_detail->log_head    = e_cnt;                                                                    \
        err_set_detail->error_count++;                                                                          \
        int fault_bit_set = eh_fault_status_get(__e_code__);                                                    \
        if(!fault_bit_set){                                                                                     \
            eh_set_trc_msg(MODULE_NUM, SUBMODULE_NUM, e_cnt, __LINE__, __e_type__ ,__e_code__);                 \
        }                                                                                                       \
        eh_fault_status_set(__e_type__, __e_code__);                                                            \


#define ERROR_SET(__e_type__, __e_code__, __detail__)do{ \
    ERROR_SET__(__e_type__, __e_code__, __detail__);     \
    fh_info_t fh_info = {.e_type = (__e_type__),         \
                     .e_code = (__e_code__),             \
                     .error_count = e_cnt,               \
                     .module_num = MODULE_NUM,           \
                     .submodule_num = SUBMODULE_NUM,     \
                     .line_num = line,                   \
                     .condition = CO_ERROR_SET};         \
    fh_fault_handle(&fh_info);                           \
} while(0)                                               \


#define ERROR_SET_NOFH(__e_type__, __e_code__, __detail__)do{ \
    __ERROR_SET(__e_type__, __e_code__, __detail__);          \
} while(0)                                                    \


#define ERROR_CLEAR(__e_type__, __e_code__)do{                                                            \
        uint16_t line =  __LINE__;                                                                        \
        error_detail_t* err_clear_detail = eh_get(MODULE_NUM, SUBMODULE_NUM);                             \
        int e_cnt = err_clear_detail->error_count  & (err_clear_detail->error_detail_log_len - 1);        \
        int fault_bit_set = eh_fault_status_get(__e_code__);                                              \
        if(fault_bit_set){                                                                                \
            eh_clr_trc_msg(MODULE_NUM, SUBMODULE_NUM, e_cnt, __LINE__, __e_type__ ,__e_code__);           \
        }                                                                                                 \
        eh_fault_status_clear(__e_type__, __e_code__);                                                    \
        fh_info_t fh_info = {.e_type = (__e_type__),                                                      \
                             .e_code = (__e_code__),                                                      \
                             .error_count = e_cnt,                                                        \
                             .module_num = MODULE_NUM,                                                    \
                             .submodule_num = SUBMODULE_NUM,                                              \
                             .line_num = line,                                                            \
                             .condition = CO_ERROR_CLR};                                                  \
        fh_fault_handle(&fh_info);                                                                        \
} while(0)                                                                                                \


#define EH_LOG_ERROR(__spec_det_name__, __arg__)do{                                                 \
    specific_error_detail_t* b = (specific_error_detail_t*)(__arg__);                               \
    int index = b->b_d.e_cnt  & (SIZEOF_ARRAY(error_detail) - 1);                                   \
    int base_detail_size = sizeof(base_error_detail_t);                                             \
    memcpy(&error_detail[index].b_d,                                                                \
           arg,                                                                                     \
           base_detail_size);                                                                       \
    if(b->specific_detail != NULL &&                                                                \
       &error_detail[index].__spec_det_name__ != NULL){                                             \
        memcpy(&error_detail[index].__spec_det_name__,                                              \
               b->specific_detail,                                                                  \
               sizeof(error_detail[index]) - base_detail_size);                                     \
    }                                                                                               \
} while(0)                                                                                          \
                                                                                       


#ifdef __cplusplus
}
#endif

#endif /* ERROR_HANDLER_H */

