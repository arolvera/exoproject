/**
 * @file    task_monitor_halo12.c
 *
 * @brief   Implementation for Halo12 specific task monitor.
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

#include "sys_init.h"
#include "mcu_include.h"
#include "task_priority.h"
#include "app.h"
#include "hwc_main.h"
#include "definitions.h"
#define DECLARE_GLOBALS
#include "ext_decl_define.h"
#include "task-monitor/component_tasks.h"

static StackType_t tc_task_stack[configMINIMAL_STACK_SIZE * 2];
static StackType_t keeper_task_stack[configMINIMAL_STACK_SIZE * 2];
static StackType_t anode_task_stack[configMINIMAL_STACK_SIZE * 2];
static StackType_t magnet_task_stack[configMINIMAL_STACK_SIZE * 2];
static StackType_t valve_task_stack[configMINIMAL_STACK_SIZE * 2];

#define HSI_NUM_CAN_RX_BUFF 1
#define HSI_CAN_RX  \
    {                                 \
        .filter_low = HSI_ID,         \
        .filter_high = HSI_ID,        \
        .filter_type = CAN_FILTER_ID, \
        .rx_mob_count = 2             \
    }

/******************************************************************************
 * Thruster Control
******************************************************************************/
static comp_task_t tc_comp_task =
    {
        //Thruster control
        .comp_func.comp_task_init = app_init,
        .comp_func.comp_task_start = app_task,
        .comp_task_stack.task_stack = tc_task_stack,
        .comp_task_stack.task_stack_size = SIZEOF_ARRAY(tc_task_stack),
        .comp_task_stack.task_prio = tskIDLE_PRIORITY,
        .comp_info.comm_intf = INTF_ALL,
        .comp_info.comm_id = 0,
        .comp_info.comp_name = "THC",
        .comp_info.start_on_boot = true,
    };

#define TC_NUM_CAN_RX_BUFF 6
#define TC_CAN_RX  \
        {                                                 \
            .filter_low = RESPONSE_PARAMETERS_ID_KEEPER,  \
            .filter_high = RESPONSE_PARAMETERS_ID_VALVE,  \
            .filter_type = CAN_FILTER_RANGE,              \
            .rx_mob_count = 2                             \
        },                                                \
        {                                                 \
            .filter_low = BROADCAST_STATE_ID_KEEPER,      \
            .filter_high = BROADCAST_STATE_ID_VALVE,      \
            .filter_type = CAN_FILTER_RANGE,              \
            .rx_mob_count = 2                             \
        },                                                \
        {                                                 \
            .filter_low = BROADCAST_VARIABLE_ID_KEEPER,   \
            .filter_high = BROADCAST_VARIABLE_ID_VALVE,   \
            .filter_type = CAN_FILTER_RANGE,              \
            .rx_mob_count = 1                             \
        },                                                \
        {                                                 \
            .filter_low = HEALTH_ID_KEEPER,               \
            .filter_high = HEALTH_ID_VALVE,               \
            .filter_type = CAN_FILTER_RANGE,              \
            .rx_mob_count = 2                             \
        },                                                \
        {                                                 \
            .filter_low = SEG_TFER_CLT_ID,                \
            .filter_high = SEG_TFER_CLT_ID | 0xF,         \
            .filter_type = CAN_FILTER_RANGE,              \
            .rx_mob_count = 2                             \
        },                                                \
        {                                                 \
            .filter_low = BL_EMCY,                        \
            .filter_high = BL_EMCY | 0xF,                 \
            .filter_type = CAN_FILTER_RANGE,              \
            .rx_mob_count = 1                             \
        }


/******************************************************************************
 * Keeper
******************************************************************************/
static comp_task_t keeper_comp_task =
    {
        //Keeper hardware component
        .comp_func.comp_task_init = hwc_init,
        .comp_func.comp_task_start = &hwc_main,
        .comp_task_stack.task_stack = keeper_task_stack,
        .comp_task_stack.task_stack_size = SIZEOF_ARRAY(keeper_task_stack),
        .comp_task_stack.task_prio = tskIDLE_PRIORITY,
        .comp_info.comm_intf = INTF_QUEUE,
        .comp_info.comm_id = COMM_ID_KEEPER,
        .comp_info.comp_name = "Keeper",
        .comp_info.start_on_boot = true,
    };
#define KEEPER_NUM_CAN_RX_BUFF 1
#define KEEPER_CAN_RX \
    {                                                \
        .filter_low = COMMAND_PARAMETERS_ID_KEEPER,  \
        .filter_high = COMMAND_PARAMETERS_ID_KEEPER, \
        .filter_type = CAN_FILTER_ID,                \
        .rx_mob_count = 1                            \
    }

/******************************************************************************
 * Anode
******************************************************************************/
static comp_task_t anode_comp_task =
    {
        .comp_func.comp_task_init = hwc_init,
        .comp_func.comp_task_start = &hwc_main,
        .comp_task_stack.task_stack = anode_task_stack,
        .comp_task_stack.task_stack_size = SIZEOF_ARRAY(anode_task_stack),
        .comp_task_stack.task_prio = tskIDLE_PRIORITY,
        .comp_info.comm_intf = INTF_CAN,
        .comp_info.comm_id = COMM_ID_ANODE,
        .comp_info.comp_name = "Anode",
        .comp_info.start_on_boot = true,
    };
#define ANODE_NUM_CAN_RX_BUFF 1
#define ANODE_CAN_RX \
    {                                                \
        .filter_low = COMMAND_PARAMETERS_ID_ANODE,   \
        .filter_high = COMMAND_PARAMETERS_ID_ANODE,  \
        .filter_type = CAN_FILTER_ID,                \
        .rx_mob_count = 1                            \
    }

/******************************************************************************
 * Magnet
******************************************************************************/
static comp_task_t magnet_comp_task =
    {
        //Magnets
        .comp_func.comp_task_init = hwc_init,
        .comp_func.comp_task_start = &hwc_main,
        .comp_task_stack.task_stack = magnet_task_stack,
        .comp_task_stack.task_stack_size = SIZEOF_ARRAY(magnet_task_stack),
        .comp_task_stack.task_prio = tskIDLE_PRIORITY,
        .comp_info.comm_intf = INTF_CAN,
        .comp_info.comm_id = COMM_ID_MAGNET,
        .comp_info.comp_name = "Magnet",
        .comp_info.start_on_boot = true,
    };
#define MAGNET_NUM_CAN_RX_BUFF 1
#define MAGNET_CAN_RX \
    {                                                \
        .filter_low = COMMAND_PARAMETERS_ID_MAGNET,  \
        .filter_high = COMMAND_PARAMETERS_ID_MAGNET, \
        .filter_type = CAN_FILTER_ID,                \
        .rx_mob_count = 2                            \
    }

/******************************************************************************
 * Valve
******************************************************************************/
static comp_task_t valve_comp_task =
    {
        //Valves
        .comp_func.comp_task_init = hwc_init,
        .comp_func.comp_task_start = &hwc_main,
        .comp_task_stack.task_stack = valve_task_stack,
        .comp_task_stack.task_stack_size = SIZEOF_ARRAY(valve_task_stack),
        .comp_task_stack.task_prio = tskIDLE_PRIORITY,
        .comp_info.comm_intf = INTF_CAN,
        .comp_info.comm_id = COMM_ID_VALVE,
        .comp_info.comp_name = "Valve",
        .comp_info.start_on_boot = true,
    };
#define VALVE_NUM_CAN_RX_BUFF 1
#define VALVE_CAN_RX \
    {                                                \
        .filter_low = COMMAND_PARAMETERS_ID_VALVE,   \
        .filter_high = COMMAND_PARAMETERS_ID_VALVE,  \
        .filter_type = CAN_FILTER_ID,                \
        .rx_mob_count = 2                            \
    }


/******************************************************************************
 * ECPK
******************************************************************************/
#define ECPK_NUM_COMP_TASK 2
static comp_task_group_t comp_task_group_ecpk =
    {
        .comp_tasks_list.comp_tasks = {&tc_comp_task, &keeper_comp_task},
        .comp_tasks_list.num_comp_tasks = ECPK_NUM_COMP_TASK,
        .can_init =
            {
                .rx_bufs = {TC_CAN_RX, HSI_CAN_RX},
                .num_rx_bufs = TC_NUM_CAN_RX_BUFF + HSI_NUM_CAN_RX_BUFF,
                .tx_mob_count = 1
            },
        .res_id = ECPK_RESID,
    };

/******************************************************************************
 * ACP
******************************************************************************/
#define ACP_NUM_COMP_TASK 1
static comp_task_group_t comp_task_group_acp =
    {
        .comp_tasks_list.comp_tasks = {&anode_comp_task},
        .comp_tasks_list.num_comp_tasks = ACP_NUM_COMP_TASK,
        .can_init =
            {
                .rx_bufs =  {ANODE_CAN_RX, HSI_CAN_RX},
                .num_rx_bufs = ANODE_NUM_CAN_RX_BUFF + HSI_NUM_CAN_RX_BUFF,
                .tx_mob_count = 4
            },
        .res_id = ACP_RESID,
    };

/******************************************************************************
 * MVCP
******************************************************************************/
#define MVCP_NUM_COMP_TASK 2
static comp_task_group_t comp_task_group_mvcp =
    {
        .comp_tasks_list.comp_tasks = {&magnet_comp_task, &valve_comp_task},
        .comp_tasks_list.num_comp_tasks = MVCP_NUM_COMP_TASK,
        .can_init = {
            .rx_bufs = {MAGNET_CAN_RX, VALVE_CAN_RX, HSI_CAN_RX},
            .num_rx_bufs = MAGNET_NUM_CAN_RX_BUFF + VALVE_NUM_CAN_RX_BUFF + HSI_NUM_CAN_RX_BUFF,
            .tx_mob_count = 4
        },
        .res_id = MVCP_RESID,
    };


/******************************************************************************
 * Just system_control
******************************************************************************/
#define SYSTEM_CONTROL_NUM_COMP_TASK 1
static comp_task_group_t comp_task_group_system_control =
    {
        .comp_tasks_list.comp_tasks = {&tc_comp_task},
        .comp_tasks_list.num_comp_tasks = SYSTEM_CONTROL_NUM_COMP_TASK,
        .can_init =
            {
                .rx_bufs = {TC_CAN_RX },
                .num_rx_bufs = TC_NUM_CAN_RX_BUFF,
                .tx_mob_count = 4
            },
        .res_id = SYS_CTRL_RESID,
    };

/******************************************************************************
 * All hardware control components
******************************************************************************/
#define HARDWARE_CONTROL_NUM_COMP_TASK 4
static comp_task_group_t comp_task_group_hardware_control =
    {
        .comp_tasks_list.comp_tasks = { &keeper_comp_task, &anode_comp_task, &magnet_comp_task, &valve_comp_task},
        .comp_tasks_list.num_comp_tasks = HARDWARE_CONTROL_NUM_COMP_TASK,
        .can_init =
            {
                //Total of mobs must be less than 15
                .rx_bufs = {KEEPER_CAN_RX, ANODE_CAN_RX, MAGNET_CAN_RX, VALVE_CAN_RX, HSI_CAN_RX},
                .num_rx_bufs = KEEPER_NUM_CAN_RX_BUFF + ANODE_NUM_CAN_RX_BUFF + MAGNET_NUM_CAN_RX_BUFF +
                    VALVE_NUM_CAN_RX_BUFF + HSI_NUM_CAN_RX_BUFF,
                .tx_mob_count = 2
            },
        .res_id = HRD_CTRL_RESID,
    };

/******************************************************************************
 * Run all components together
******************************************************************************/
#define ALL_COMP_NUM_COMP_TASK 5
static comp_task_group_t comp_task_group_all_comp =
    {
        .comp_tasks_list.comp_tasks = {&tc_comp_task,
                                        &keeper_comp_task,
                                        &anode_comp_task,
                                        &magnet_comp_task,
                                        &valve_comp_task},
        .comp_tasks_list.num_comp_tasks = ALL_COMP_NUM_COMP_TASK,
        .can_init = {0},
//            {
//                //Total of mobs must be less than 15
//                .rx_bufs = {KEEPER_CAN_RX, ANODE_CAN_RX, MAGNET_CAN_RX, VALVE_CAN_RX, HSI_CAN_RX},
//                .num_rx_bufs = KEEPER_NUM_CAN_RX_BUFF + ANODE_NUM_CAN_RX_BUFF + MAGNET_NUM_CAN_RX_BUFF +
//                    VALVE_NUM_CAN_RX_BUFF + HSI_NUM_CAN_RX_BUFF,
//                .tx_mob_count = 2
//            },
        .res_id = ALL_CTRL_RESID,
    };


/**
 * Given a comm_id. Return communication interface
 * @param comm_id
 * @return COMPONENT_INTF
 */
COMPONENT_INTF comp_task_get_intf(uint32_t comm_id)
{
    uint8_t component_id = 0xF & comm_id;
    COMPONENT_INTF intf = 0;

    if(comm_id == HSI_ID) {
        //If HSI message send on all interfaces
        if(system_res_id == ECPK_RESID) {
            intf = INTF_ALL;
        }else {
            //If system ctrl comp running alone just use can.
            intf = INTF_QUEUE;
        }
    }else {
        switch(component_id) {
            case COMM_ID_KEEPER:
                intf = keeper_comp_task.comp_info.comm_intf;
                break;
            case COMM_ID_ANODE:
                intf = anode_comp_task.comp_info.comm_intf;
                break;
            case COMM_ID_MAGNET:
                intf = magnet_comp_task.comp_info.comm_intf;
                break;
            case COMM_ID_VALVE:
                intf = valve_comp_task.comp_info.comm_intf;

                break;
            default:
                intf = INTF_INVALID;
        }
    }

    return intf;
}

comp_can_init_t *comp_task_group_get_can_rx_bufs(void)
{
    switch(system_res_id) {
        case ECPK_RESID:
            return &comp_task_group_ecpk.can_init;
        case ACP_RESID:
            return &comp_task_group_acp.can_init;
        case MVCP_RESID:
            return &comp_task_group_mvcp.can_init;
        case SYS_CTRL_RESID:
            return &comp_task_group_system_control.can_init;
        case HRD_CTRL_RESID:
            return &comp_task_group_hardware_control.can_init;
        case ALL_CTRL_RESID:
            return &comp_task_group_all_comp.can_init;
        default:
            return NULL;
    }
}

const comp_task_list_t *comp_task_list_get(void)
{
    switch(system_res_id) {
        case ECPK_RESID:
            return &comp_task_group_ecpk.comp_tasks_list;
        case ACP_RESID:
            return &comp_task_group_acp.comp_tasks_list;
        case MVCP_RESID:
            return &comp_task_group_mvcp.comp_tasks_list;
        case SYS_CTRL_RESID:
            return &comp_task_group_system_control.comp_tasks_list;
        case HRD_CTRL_RESID:
            return &comp_task_group_hardware_control.comp_tasks_list;
        case ALL_CTRL_RESID:
            return &comp_task_group_all_comp.comp_tasks_list;
        default:
            return NULL;
    }
}

void comp_power(uint8_t resid, bool on)
{
    comp_task_t *comp;

    switch(resid) {
        case ECPK_RESID:
            comp = &keeper_comp_task;
            if(on){
                comp->comp_task_stack.task_handle =
                    xTaskCreateStatic(
                        comp->comp_func.comp_task_start,
                        comp->comp_info.comp_name,
                        comp->comp_task_stack.task_stack_size,
                        &comp->comp_info.comm_id,
                        comp->comp_task_stack.task_prio,
                        comp->comp_task_stack.task_stack,
                        &comp->comp_task_stack.task_buf
                                     );
            }else{
                if(comp->comp_task_stack.task_handle){
                    vTaskDelete(comp->comp_task_stack.task_handle);
                }
            }
            break;
        case ACP_RESID:
        case MVCP_RESID:
//            gpio_set(&gpio_mcu_shutdown, on);
            break;
        case ALL_CTRL_RESID:
            // This case likely only used when running as a simulator on x86?
            for(int i = 0; i < comp_task_group_hardware_control.comp_tasks_list.num_comp_tasks; i++){
                comp = &keeper_comp_task;
                if(on){
                    comp->comp_task_stack.task_handle =
                        xTaskCreateStatic(
                            comp->comp_func.comp_task_start,
                            comp->comp_info.comp_name,
                            comp->comp_task_stack.task_stack_size,
                            &comp->comp_info.comm_id,
                            comp->comp_task_stack.task_prio,
                            comp->comp_task_stack.task_stack,
                            &comp->comp_task_stack.task_buf
                        );
                }else{
                    if(comp->comp_task_stack.task_handle){
                        vTaskDelete(comp->comp_task_stack.task_handle);
                    }
                }
            }
        default:
            break;
    }
}

void comp_pre_init()
{
    // Init system
    sys_init();
    /* GPIO perihperal is init'd at the same time as EBI, because EBI uses
        GPIO pins for memory accesses */
    gpio_init(&gpio_thruster_present, GPIO_OUTPUT);
    gpio_init(&gpio_active_led, GPIO_OUTPUT);
    gpio_set(&gpio_active_led, 1);
    gpio_set(&gpio_thruster_present, 1);

 }

uint32_t comp_read_res_id(void)
{
    uint16_t rid0 = gpio_rd(&gpio_resid0); // F5
    uint16_t rid1 = gpio_rd(&gpio_resid1); // F4
    uint16_t rid2 = gpio_rd(&gpio_resid2); // F3
    uint16_t rid3 = gpio_rd(&gpio_resid3); // F2
    uint16_t rid4 = gpio_rd(&gpio_resid4); // F10

    return (uint32_t)((rid4 << 4) | (rid3 << 3) | (rid2 << 2) | (rid1 << 1) | rid0);
}
