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
#include "fault_handler.h"
//#include "co_task.h"
#include "client-control/client_control.h"
#include "client-control/client_lockout.h"
#include "health/health.h"
#include "trace/trace.h"
#include "utils/macro_tools.h"
#include "utils/stack.h"
#include "co_emcy.h"

//extern CO_NODE tc_node;

#define MAX_MODULE_ERROR_CODES 5

typedef void (*FAULT_HANDLER)(int);

void fault_handler_nmt_emergency_shutdown(int arg)
{
    (void)arg;
//    canopen_task_nmt_error_state();
}

void fault_handler_lockout(int arg)
{
    client_lockout_queue(arg);
}

void fault_handler_thruster_shutdown(int arg)
{
    (void)arg;
    client_shutdown();
}

/* Function table handles funtion numbers 2 and greater.  Function numbers
 * 0 and 1 are reserved */
static FAULT_HANDLER fault_handler_function_table[] = { fault_handler_nmt_emergency_shutdown,
                                                        fault_handler_lockout,
                                                        fault_handler_thruster_shutdown,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        NULL };


typedef struct{
    uint8_t fault_num;
    uint8_t fault_handler;
}fault_handler_table_t;

#define MAX_FAULT_HANDLERS 164

/* Need register function for more specific error codes */
fault_handler_table_t fault_handlers[MAX_FAULT_HANDLERS];

/**
 * Subscribe register a fault handling method to an error code
 * 
 * 
 * @param error code to attach handler too
 * @param handler to attach
 * @return error status
 */                               
int fh_fault_handlers_register(uint8_t fault_num, uint8_t fault_handler)
{
    int err = 0;
    if(fault_num < MAX_FAULT_HANDLERS                             && 
       fault_handler < SIZEOF_ARRAY(fault_handler_function_table) && 
       fault_handlers[fault_num].fault_num == 0)
    {
        fault_handlers[fault_num].fault_num = fault_num;
        fault_handlers[fault_num].fault_handler = fault_handler;
    } else {
        TraceE3(TrcMsgErr3, "Invalid fault number: %d or fault handler: %d",
                fault_num,fault_handler,0,0,0,0);
        
        
        err = __LINE__;
    }
    return err;
}

/*
 * check whether the fault code has already been assigned a fault handler
 * @param Fault number to check
 * @return fault number.  0 indicates no fault handler assigned yet
 */
int fh_fault_handlers_registry_check(uint8_t fault_num)
{
    return fault_handlers[fault_num].fault_num;
}


/**
 * Subscribe register a fault handling method to an error code
 * 
 * 
 * @param pointer to all info fault handler related
 * @return error status
 */ 
int fh_fault_handle(fh_info_t* fh_info)
{
    int err = 0;
    
    CO_EMCY_USR tcEmcyUsr = {0};
    tcEmcyUsr.Emcy[0] = fh_info->e_type;
    tcEmcyUsr.Emcy[1] = fh_info->e_code;
    *((uint16_t*)&tcEmcyUsr.Emcy[2]) = fh_info->line_num;
    tcEmcyUsr.Emcy[4] = fh_info->error_count;

    tcEmcyUsr.Hist = fh_info->e_code;

    uint8_t function_number = fault_handlers[fh_info->e_code].fault_handler;

    if(function_number < SIZEOF_ARRAY(fault_handler_function_table) && function_number > 0){
        /*
         * If function number is 0, don't do anything, if it's greater than zero
         *  always set an emcy
         */
        if(fh_info->condition){

//            COEmcySet(&tc_node.Emcy, fh_info->e_code, &tcEmcyUsr);

           /*
            * If function number is greater than 1, take action
            */
           if(function_number > 1){
               FAULT_HANDLER fp = fault_handler_function_table[function_number - 2];
               if(fp != NULL) {
                   fp(fh_info->e_code);
               } else {
                   TraceE3(TrcMsgErr3, "No fault handler assigned to:%d",
                           function_number - 2, 0,0,0,0,0);
               }
           }
        } else {
            /*
             * Make sure all errors of this type are cleared before emcy clearing them
             */
//            COEmcyClr(&tc_node.Emcy, fh_info->e_code);

        }
    }
    
    return err;
}


/**
 * Reconfigures fault handler assigned to error code
 * 
 * 
 * @param error code to reconfigure
 * @param fault handler to assign to error code
 * @param choose read or right action. NOTE: if action is FH_READ new_fault_handler param is ignored
 * @return error status
 */ 
int fh_fault_config(uint8_t e_code, uint8_t new_fault_handler, fh_action_t action)
{
    int fh_value = -1;

    if(new_fault_handler < SIZEOF_ARRAY(fault_handler_function_table) && 
       e_code < SIZEOF_ARRAY(fault_handlers)                          && 
       action == FH_WRITE)
    {
       fault_handlers[e_code].fault_handler = new_fault_handler; 
       /* 0 indicates successful write */
       fh_value = 0;
    } else if((e_code < SIZEOF_ARRAY(fault_handlers)) && (action == FH_READ)){
        /* Not -1 indicates successful read */
        fh_value = fault_handlers[e_code].fault_handler;
    }
    
    return fh_value;
}


/**
 * Return fault handler configuration array
 * 
 * 
 * @param pointer to fill in size of configuration array
 * @return error status
 */ 
void* fh_fault_config_get(unsigned int* size)
{
    *size = sizeof(fault_handlers);
    return fault_handlers;
}