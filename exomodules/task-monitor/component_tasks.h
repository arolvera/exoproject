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

#ifndef MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXOMODULES_TASK_MONITOR_COMPONENT_TASKS_H_
#define MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXOMODULES_TASK_MONITOR_COMPONENT_TASKS_H_
#include <stdbool.h>
#include <stdint.h>
#include "osal/osal.h"
#include "can/hal_can.h"
#include "ext_decl_define.h"
#include "gpio/hal_gpio.h"

#define MAX_COMP_TASKS 10

typedef enum {
  INTF_CAN,
  INTF_QUEUE,
  INTF_ALL,
  INTF_INVALID,
} COMPONENT_INTF;

typedef void(comp_task_init_f)(void);    //component Inits hardware and values
typedef void(comp_task_start_f)(void *pv); //component start task func
typedef struct {
  comp_task_init_f *comp_task_init;
  comp_task_start_f *comp_task_start;
} comp_funcs_t;

typedef struct {
    OSAL_STATIC_STACK_TYPE task_buf;
    OSAL_STATIC_STACK_BUFFER_TYPE  *task_stack;      //user defines stack
    int          task_stack_size;
    int          task_prio;
    OSAL_TASK_HANDLE_TYPE task_handle;
} comp_task_stack_t;

//Every component(thruster control, hardware component) needs an init, shutdown, and task start func.
//Other information for setting up can peripheral
typedef struct {
  uint16_t comm_id;                        //can communication id
  const COMPONENT_INTF comm_intf;                //communication interface
  const bool start_on_boot;                //start component task on boot
  const char comp_name[configMAX_TASK_NAME_LEN];
} comp_info_t;

typedef struct {
  const comp_funcs_t comp_func;
  comp_task_stack_t comp_task_stack;
  comp_info_t comp_info;
} comp_task_t;

//List of component tasks
typedef struct {
    int num_comp_tasks;
    comp_task_t *comp_tasks[MAX_COMP_TASKS];
}comp_task_list_t;

#define MAX_RX_BUFFS 16
//Control process board can_init. A board can have multiple control process(components).
typedef struct {
  const int tx_mob_count;
  const int num_rx_bufs;
  can_rx_buffer_t rx_bufs[MAX_RX_BUFFS];
}comp_can_init_t;

//Component tasks grouped together by resistor id
typedef struct {
  const comp_task_list_t comp_tasks_list;
  comp_can_init_t can_init;
  uint16_t res_id;                    //board resistor id
}comp_task_group_t;

EXT_DECL uint32_t system_res_id
#if InitVar
= -1
#endif
;

/**
 * Defined by project user. Given a res id return a list of component tasks tied to that id.
 * @param res_id
 * @return grouping of component tasks
 */
extern const comp_task_list_t *comp_task_list_get(void);
extern comp_can_init_t *comp_task_group_get_can_rx_bufs(void);//Get rx buffers for a resistor id
extern COMPONENT_INTF comp_task_get_intf(uint32_t comm_id);
extern void comp_power(uint8_t resid, bool on);
extern void comp_pre_init(void);
extern uint32_t comp_read_res_id(void);

//Init can peripheral with comp tasks group
int comp_task_can_init(OSAL_QUEUE_HANDLE_TYPE *rx_q_handle);

#endif//MASTER_IMAGE_CONSTRUCTOR_HALO12_VA41630_KRYPTON_SILVER_BOEING_EXOMODULES_TASK_MONITOR_COMPONENT_TASKS_H_
