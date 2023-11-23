//
// Created by marvin on 9/20/22.
//

#ifndef VORAGO_DEV_HALO12_SRC_COMMON_INCLUDE_DEFINITIONS_H_
#define VORAGO_DEV_HALO12_SRC_COMMON_INCLUDE_DEFINITIONS_H_
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "ext_decl_define.h"
#include "device.h"
#include "cmsis/cmsis_compiler.h"
#include "serial/hal_serial.h"
#include "ebi/hal_ebi.h"
#include "gpio/hal_gpio.h"

#ifdef FREE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "osal/osal.h"
#include "task_priority.h"
#endif

/*
 * hardware components have timers tied to specific pins. Those timers will
 * not be available. These are the available times.
 */
typedef enum {
  SYS_TICK_TIMER_NUM = 0,
} SOFTWARE_TIMERS;

#define EBI_SRAM_UPDATE_SIZE (0x80000) //500KB of update space for hrd ctrl

//Defined by linker script. Defines addr region for ebi external memory
extern const uint32_t EBI_SRAM_START_ADDR[];
extern const uint32_t EBI_SRAM_END_ADDR[];

//Must be const so ebi can be init before .data and .bss moved to ram or ext sram
EXT_DECL const ebi_init_t ebi_bus_init[]
#if InitVar
    = {
#ifndef __x86_64__
        { .start_addr = 0xFF, .end_addr = 0xFF, .read_cycles = 6, .write_cycles = 6, .turn_around_cycles = 6 }, //not used, cannot disable, so map end of range
        { .start_addr = (uint32_t)EBI_SRAM_START_ADDR, .end_addr = (uint32_t)EBI_SRAM_END_ADDR, .read_cycles = 3, .write_cycles = 3, .turn_around_cycles = 3, .en16bitmode=0},
#endif
        { .start_addr = 0xFF, .end_addr = 0xFF, .read_cycles = 6, .write_cycles = 6, .turn_around_cycles = 6 }, //not used, cannot disable, so map end of range
        { .start_addr = 0xFF, .end_addr = 0xFF, .read_cycles = 6, .write_cycles = 6, .turn_around_cycles = 6 }, //not used, cannot disable, so map end of range
    }
#endif
;

//External uart init struct
EXT_DECL const uart_init_t ext_uart_init
#if InitVar
    = {
        .uart_if_id = UART_IF_ID_0, .baud = UART_BAUD_115200, .rx_irq_enable = true, .tx_irq_enable = false,
        .tx_irq_priority = 5, .rx_irq_priority = 5,
        .rx_interrupt_level = sizeof(message_t), .tx_interrupt_level = sizeof(message_t),
#if FREE_RTOS
        .rx_task_prio = UART_RX_TASK_PRIO, .tx_task_prio = UART_TX_TASK_PRIO,
#endif

      }
#endif
;

EXT_DECL hal_gpio_t gpio_mcu_shutdown
#if InitVar
    = {(uint32_t *)VOR_PORTA_BASE, 8}
#endif
;

EXT_DECL hal_gpio_t gpio_thruster_present
#if InitVar
 = {(uint32_t  *)VOR_PORTA_BASE, 5}
#endif
;

EXT_DECL hal_gpio_t gpio_active_led
#if InitVar
    = {(uint32_t *)VOR_PORTA_BASE, 15}
#endif
;

/*
 * GPIO     PF10  PF5 PF4 PF3 PF2
 * RESID      4    3   2   1   0
 * ------------------------------------
 * ECPK       0    0   1   0   1
 * MVCP       0    1   0   1   0
 * ACP        0    1   0   1   1
 */
/* Actual measured values on first board
 * GPIO     PF10  PF5 PF4 PF3 PF2
 * RESID      4    3   2   1   0
 * ------------------------------------
 * ECPK       1    1   1   1   1
 * MVCP       0    1   1   0   1
 * ACP        0    0   1   0   1
 */
EXT_DECL hal_gpio_t gpio_resid0
#if InitVar
    = {(uint32_t *)VOR_PORTF_BASE, 5}
#endif
;

EXT_DECL hal_gpio_t gpio_resid1
#if InitVar
    = {(uint32_t *)VOR_PORTF_BASE, 4}
#endif
;

EXT_DECL hal_gpio_t gpio_resid2
#if InitVar
    = {(uint32_t *)VOR_PORTF_BASE, 3}
#endif
;

EXT_DECL hal_gpio_t gpio_resid3
#if InitVar
    = {(uint32_t *)VOR_PORTF_BASE, 2}
#endif
;

EXT_DECL hal_gpio_t gpio_resid4
#if InitVar
    = {(uint32_t *)VOR_PORTF_BASE, 10};
#endif
;
// Original values ToDo: go back to these?
//#define ECPK_RESID (0x05) //Engine control process(thruster control) and keeeper
//#define MVCP_RESID (0x0A) //Mangnet and Valves control process
//#define ACP_RESID  (0x0B) //Anode control process
// Values measured on first board stack
#define ECPK_RESID (0x1F) //Engine control process(thruster control) and keeeper
#define MVCP_RESID (0x0D) //Mangnet and Valves control process
#define ACP_RESID  (0x05) //Anode control process
//Debug resids
#define SYS_CTRL_RESID  (0x01)
#define HRD_CTRL_RESID  (0x02)
#define ALL_CTRL_RESID  (0x03)

#endif //VORAGO_DEV_HALO12_SRC_COMMON_INCLUDE_DEFINITIONS_H_
