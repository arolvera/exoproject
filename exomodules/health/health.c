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
/* 
 * File:   health.c
 * Author: jmeyers
 *
 * Created on April 29, 2021, 5:09 AM
 */
#include "health.h"
#include "can/hal_can.h"
#include "definitions.h"
#include "trace/trace.h"
#include "msg-handler/msg_handler.h"

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define HEALTH_SYNC_ID 0x80
/* This is CAN address space reserved for client health messages. Each client
 * has a PDO base ID + its CAN ID */
#define HEALTH_CLIENT_START_ID 0x480
#define HEALTH_CLIENT_MAX_ID   0x4FF

/* Callback function to handle HSI messages */
static int health_msg_callback(message_t *msg);

/* Call back pointer & message IDs to register for client messages */
msg_callback_t health_node = {
    .node = {
        .range_low  = HEALTH_CLIENT_START_ID,
        .range_high = HEALTH_CLIENT_MAX_ID,
        .left = NULL,
        .right = NULL,
    },
    .cb = health_msg_callback,
};
    
/* Number of health and status messages */
#define HEALTH_SYNC_COUNT 3

#define MCU_ID_TO_BITMASK(_MCU_ID_) (1 << _MCU_ID_)
#define CAN_ID_TO_MCU_ID( _CAN_ID_) (_CAN_ID_ - HEALTH_ID_BASE)

/* Current health sync number */
static uint16_t current_tick = 0;

/* HSI is valid when all ticks have been received at 'threshold' number of times */
#define HSI_VALID_THRESHOLD 2
static int num_hsi_valid = 0;

/* Maximum Number of devices supported */
#define HSI_MAX_DEV_ID 6
/* If HSI_MAX_DEV_ID Exceeds 8 this will need to be updated also */
typedef union {
    uint64_t history;
    uint32_t history32[2];
    uint8_t array[HSI_MAX_DEV_ID];
} missing_history_t;
missing_history_t missing;

/* Device HSI go directly into the HSI Array.  So, the Anode CAN ID & Component
 * ID is '2', therefore it is in HSI Array position 2.   This convenience may
 * come at the cost of blank spots in the array, depending on system
 * configuration.  However, it is just one pointer (4 bytes) per blank spot,
 * which is liveable.
 */
#define HSI_ARRAY_SIZE (HSI_MAX_DEV_ID + 1)
health_table_entry_t** dev_health[HSI_ARRAY_SIZE];

hsi_conv_func_t hsi_conv_func[HSI_ARRAY_SIZE];

/* Devices received vs. expected during 1 HSI tick */
static uint32_t devices_checked_in = 0;
/* Devices that checked in during the last HSI tick.  This can be used for
 * external callers who need to make sure a device is checking in.  You cannot
 * use the same variable used for devices checking in on the current tick,
 * because you will be racing the devices and might erroneously conclude a
 * device is not checking in */
static uint32_t devices_last_checked_in = 0;
static uint32_t devices_expected = 0;
static uint32_t devices_consecutive_misses = 0;

/* The threshold where missing devices cause an error */
#define DEVICE_MISSING_HSI_THRESHOLD 16

#define      health_timer_TASK_STACK_SIZE ( configMINIMAL_STACK_SIZE )
#define      health_timer_TASK_PRIORITY   ( tskIDLE_PRIORITY )
StackType_t  health_timer_stack[health_timer_TASK_STACK_SIZE];
StaticTask_t health_timer_stack_buffer;

/**
 * Verify that all Client MCUs responded with the Health info.  Set flags and
 * raise alarms if one or more are missing
 * 
 * @param sync_number the health sync number to check
 */
static void health_check_sync(int sync_number)
{
    if(Health_Tick_Enabled && devices_checked_in != devices_expected) {
        devices_consecutive_misses++;
        /* Get the missing mask (mm) */
        uint32_t mm = (devices_checked_in ^ devices_expected);
        int pos = 0;
        while(mm) {
            if(mm & 1) {
                missing.array[pos]++;
            }
            pos++;
            mm = mm >> 1;
        }
        TraceE2(TrcMsgErr2, "Missing HSI s:%d e:%02x r:%02x h:%08x:%08x c:%d",
                current_tick, devices_expected, devices_checked_in,
                missing.history32[1], missing.history32[0],
                devices_consecutive_misses);
    } else {
        devices_consecutive_misses = 0;
    }
    devices_last_checked_in = devices_checked_in;
    devices_checked_in = 0;
}

/**
 * Check if HSI for the given device is up-to-date
 * @param dev_id device id
 * @return 1 if device checked in on the last tick, 0 if not.
 */
int health_device_hsi_recieved(component_type_t dev_id)
{
    return devices_last_checked_in & MCU_ID_TO_BITMASK(dev_id) ? 1:0;
}
/**
 * Return the number of consecutive HSI update misses.  Any MCU can miss and it
 * increments the count.  The count is cleared the first time ALL MCUs check
 * back in.
 * @return number of consecutive HSI misses
 */
uint32_t health_device_misses_get(void)
{
    return devices_consecutive_misses;
}

/**
 * Copy health info into buffer - Return number of bytes copied
 * @param entry pointer to health entry
 * @param buf buffer to store data
 * @return number of bytes copied into buffer
 */
static int health_copy_entry(health_table_entry_t *entry, uint8_t *buf, uint8_t mcu_id)
{
    int bytes_copied = 0;
    uint8_t *pBuf = buf;
    health_table_entry_t *pEntry = entry;
    int i = 0;
    uint8_t arg_array[4] = {0};
    
    while(pEntry->size != HEALTH_ENTRY_SIZE_EOL) {
        switch(pEntry->size) {
            case HEALTH_ENTRY_SIZE_1:
                *((uint8_t *)(pEntry->data)) = *((uint8_t*) pBuf);
                arg_array[i++] = *((uint8_t *)(pEntry->data)) = *((uint8_t*) pBuf); 
                break;
            case HEALTH_ENTRY_SIZE_2:
                *((uint16_t *)(pEntry->data)) = *((uint16_t*) pBuf);
                arg_array[i++] = *((uint16_t *)(pEntry->data)) = *((uint16_t*) pBuf); 
                break;
            case HEALTH_ENTRY_SIZE_4:
                *((uint32_t *)(pEntry->data)) = *((uint32_t*) pBuf);
                arg_array[i++] = *((uint32_t *)(pEntry->data)) = *((uint32_t*) pBuf); 
                break;
            case HEALTH_ENTRY_SIZE_EOL:
            default:
                /* Wait what happened? */
                break;
        }
        /* Increment bytes copied */
        bytes_copied += pEntry->size;
        /* Advance the buffer pointer */
        pBuf += pEntry->size;
        /* Next! */
        pEntry++;
    }
    (void) arg_array; // In the case of Trace messages disabled, this is used
    TraceDbg(TrcMsgHSI, 
        "ID:%2x s:%2x d:%04x:%04x:%04x:%04x", 
        mcu_id, current_tick, 
        arg_array[0], arg_array[1], arg_array[2], arg_array[3]);
    
    return bytes_copied;
}

/**
 * Process a message coming back from one of the MCUs
 * @param mcu_id the mcu ID (just the mcu id, not the whole CAN ID)
 * @param data buffer containing the health data
 * @param dlc length of the message
 */
static void health_timersend_task(void *pvParameters)
{
    TickType_t xNextWakeTime;
    // Empty Sync buffer - 1 byte a sync count
    uint8_t data[8] = {0};

    // Initialize xNextWakeTime - this only needs to be done once.
    xNextWakeTime = xTaskGetTickCount();
    for (;;) {
        /* Place this task in the blocked state until it is time to run again. */
        vTaskDelayUntil(&xNextWakeTime, (Health_Tick_Millisecs/portTICK_PERIOD_MS));
        if(Health_Tick_Enabled != 0) {
            /* Verify that last set of health data accumlated successfully */
            health_check_sync(current_tick);
            if(num_hsi_valid < HSI_VALID_THRESHOLD && current_tick == (HEALTH_SYNC_COUNT - 1)) {
                num_hsi_valid++;
            }
            current_tick = (current_tick + 1) % HEALTH_SYNC_COUNT;
            data[0] = current_tick;
            send_msg(HEALTH_SYNC_ID, data, CAN_DLC_1, portMAX_DELAY);
        }
    }
}

/**
 * Process a message coming back from one of the MCUs
 * @param mcu_id the mcu ID (just the mcu id, not the whole CAN ID)
 * @param data buffer containing the health data
 * @param dlc length of the message
 */
static int health_msg_callback(message_t *msg)
{
    // Process a health message coming back from the MCUs
    // Store it by the current tick
    int err = 0;
    uint16_t mcu_id = CAN_ID_TO_MCU_ID(msg->id);
    
    if((msg->dlc != 0) && (mcu_id <= HSI_MAX_DEV_ID) && (mcu_id > 0) && (msg->dlc <= CAN_DLC_8)) {
        /* Get the pointer to the MCU's health and make sure that is not NULL
         * Then get the pointer to the MCU's tick entry and make sure
         * that is not NULL */
        health_table_entry_t **ph = dev_health[mcu_id];
        health_table_entry_t *pph = NULL;
        if(ph != NULL) {
            pph = ph[current_tick];
        }
        if(pph != NULL) {
            health_copy_entry(pph, msg->data, mcu_id);
            if (hsi_conv_func[mcu_id] != NULL){
                (*hsi_conv_func[mcu_id])(current_tick);
            }
            devices_checked_in |= MCU_ID_TO_BITMASK(mcu_id);
        }
        TraceDbg(TrcMsgHSI, "id:%d ph:%p, pph:%p dci:0x%x",
                mcu_id, ph, pph, devices_checked_in, 0,0);
    }
    return err;
}

/**
 * Register MCU to monitored by the health task
 * @param dev_id component ID of the MCU
 * @param ph pointer to the health entry array
 * @param cf pointer to the converter function
 */
void health_mcu_register(component_type_t dev_id, health_table_entry_t** ph, hsi_conv_func_t cf )
{
    int err = 0;
    if(dev_id > HSI_MAX_DEV_ID) {
        err = __LINE__;
    }
    if(!err) {
        dev_health[dev_id] = ph;
        hsi_conv_func[dev_id] = cf;
        devices_expected |= MCU_ID_TO_BITMASK(dev_id);
    }
    TraceDbg(TrcMsgHSI, "id:%d, ph:%p, cf:%p, err:%d",
                dev_id, ph, cf, err,0,0);
}

/**
 * Disable HSI monitoring for the MCU
 * @param dev_id component ID
 */
void health_mcu_disable(component_type_t dev_id)
{
    int err = 0;
    if(dev_id > HSI_MAX_DEV_ID) {
        err = __LINE__;
    }
    if(!err) {
        dev_health[dev_id] = NULL;
        devices_expected &= ~(1 << dev_id);
    }
}

/**
 * Enable health and status message
 * @param enable true to enable, false otherwise
 */
void health_enable(bool enable)
{
    /* If being enabled & not already enabled - reset tick & prime the pump */
    if(enable && !Health_Tick_Enabled) {
        missing.history = 0;
        current_tick = 0;
        num_hsi_valid = 0;
        /* Pretend they all came in the first time, to avoid any startup race conditions */
        devices_checked_in = devices_expected;
    }
    devices_consecutive_misses = 0;
    Health_Tick_Enabled = enable;
}

/**
 * Check if health message are enabled
 * @return true if enabled, false otherwise
 */
bool health_enabled(void)
{
    return Health_Tick_Enabled;
}

bool health_valid(void)
{
    return num_hsi_valid >= HSI_VALID_THRESHOLD;
}

/**
 * Initialize health and status check
 * @param tick Provide a pointer to the control the frequency of health and
 *          status messages. The frequency will be control external to this
 *          module.  The pointer passed here will be use for the life of the
 *          health task, so it must be in static memory
 */
void health_init()
{
    xTaskCreateStatic(health_timersend_task, "Health Sync Task",
            health_timer_TASK_STACK_SIZE, 0, health_timer_TASK_PRIORITY,
            health_timer_stack, &health_timer_stack_buffer);

    msg_handler_register_callback(&health_node);
}
