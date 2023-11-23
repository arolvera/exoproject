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
#include <string.h>
#include "checksum.h"
#include "cmd/command_magnets.h"
#include "error/error_handler.h"
#include "health/health.h"// Health Enable
#include "keeper/control_keeper.h"
#include "setpoint/control_setpoint.h"// Throttle setpoint
#include "storage/storage_memory_interface.h"
#include "throttle/control_throttle.h"// Throttle lookup
#include "trace/trace.h"              // Trace messages
#include "control_anode.h"
#include "client-control/client_control.h"
#include "client-control/client_error_details.h"
#define DECLARE_GLOBALS               //declare sequence_condition_sequence
#include "ext_decl_define.h"
#include "seq_cond.h"

#define ENABLE_DEBUG 1
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_CONDITION_SUBMODULE

#define cond_TASK_PRIORITY   (tskIDLE_PRIORITY + 2)
#define cond_TASK_STACK_SIZE (configMINIMAL_STACK_SIZE * 3 )
static TaskHandle_t cond_task_handle;
static StackType_t  cond_task_stack[cond_TASK_STACK_SIZE];
static StaticTask_t cond_TaskBuffer;

/* This is the outside world's view of what is happening */
static uint32_t condition_start_step   = 0;
static uint32_t condition_current_step = 0;
static uint32_t condition_step_status  = CONDITION_RUN_STAT_IDLE;

static SemaphoreHandle_t seq_semaphore;
/* This pointer is used to tell if the semaphore has been initialized.  It
 * remains NULL until the seq_semaphore is initialized, then it is set to point
 * at that variable */
static SemaphoreHandle_t *pseq_semaphore = NULL;
static StaticSemaphore_t seq_semaphore_buffer;

#define CONDITION_NVM_SIZE 512

static bool condition_status_initialized = false;

#pragma pack(push, 1)
typedef union {
    uint8_t data[CONDITION_NVM_SIZE];
    condition_stat_t condition_status[THROTTLE_COND_COUNT];
} condition_nvm_t;
#pragma pack(pop)

/**
 * 
 * @return number of sequences
 */
int ctrl_condition_num_sequence_get(void)
{
    return THROTTLE_COND_COUNT;
}

/**
 * Get Get a reference to a sequences user config var
 * @param el - sequence number
 */
ctrl_cond_seq_ucv_t *ctrl_condition_seq_ucv_get(uint32_t el)
{
    uint32_t seq_el = (el <= THROTTLE_COND_COUNT) ? el:THROTTLE_COND_COUNT;
    return &sequence_condition_sequences[seq_el].seq_ucv;
}

static condition_nvm_t cnvm;
static uint32_t *pcurrent_cond_stat = &cnvm.condition_status[0].seq_stat_cond;
/* Pointer to the last 16 bit, where the crc is stored */
static uint16_t *cnvm_read_crc = (uint16_t *)&cnvm.data[sizeof(cnvm.data) - sizeof(uint16_t)];

static uint16_t ctrl_condition_nvm_crc(void)
{
    return crc_16(cnvm.data, sizeof(cnvm.data) - sizeof(uint16_t));
}

/**
 * saves cond_stats to memory
 * @return 
 */
static int ctrl_condition_status_save(void)
{
    int err = 0;
    int fd = -1;
    char *filename = CONDITION_STATUS;
    
    *cnvm_read_crc = ctrl_condition_nvm_crc();
    
    err = storage_memory_open(filename, O_WRONLY);
    if(err >= 0) {
        fd = err;
        int size = sizeof(cnvm.data);
        err = storage_memory_write(fd, cnvm.data, size);
        if(err == size) {
            err = 0;
        }
    }
    if(fd >= 0) {
        storage_memory_close(fd);
    }
    return err;
}

/**
 * read cond stat from mem
 * @return 
 */
static int ctrl_condition_status_read(void)
{
    int err = 0;
    int fd = -1;
    char *filename = CONDITION_STATUS;
    
    if(!condition_status_initialized) {
        err = storage_memory_open(filename, O_RDONLY);
        if(err >= 0) {
            fd = err;
            int size = sizeof(cnvm.data);
            err = storage_memory_read(fd, cnvm.data, size);
            if(err == size) {
                err = 0;
            }
        }
        if(!err) {
            uint16_t  calc_crc =  ctrl_condition_nvm_crc();
            if(calc_crc != *cnvm_read_crc) {
                err = __LINE__;
                memset(cnvm.data, 0, sizeof(cnvm.data));
            }
        }
        if(fd >= 0) {
            storage_memory_close(fd);
        }
        /* Good read or zeroed either way */
        condition_status_initialized = true;
    }
    return err;
}

/**
 * Clears all sequence stats
 */
int ctrl_condition_status_clear(void)
{
    int err; 
    bzero(cnvm.data, sizeof(cnvm.data));
    err = ctrl_condition_status_save();
    if(!err){
       err = ctrl_condition_status_read();
    }
    return err;
}

int ctrl_condition_can_run(TCS_CO_MODE required_state)
{
    int err = 0;
    if((Thruster_Control_State != required_state)) {
        err = __LINE__;
    }
    if(err && Thruster_Control_State == TCS_CONDITIONING) {
        /* If not in the required state, see if we are already conditioning,
         * and then deduce further if we can take the next step
         */
        if(required_state == TCS_STANDBY) {
            /* No pre-req really, just do it */
            err = 0;
            
        } else if(required_state == TCS_READY_MODE) {
            int is_running = ctrl_keeper_isrunning();
            if(is_running) {
                err = 0; /* Keeper is running, safe to run the next step */
            }
        }
    }
    return err;
}

static int ctrl_condition_monitor_magnets(uint32_t monitor_ms, condition_stat_t *stat)
{
    int err = 0;
    
    uint32_t monitor_loops = monitor_ms/COND_MONITOR_WAIT_MS;
    uint32_t nloops = 0;
    uint32_t ms_since_since_last_info = 0;
    
    /* Initialize xNextWakeTime - this only needs to be done once. */
    TickType_t xNextWakeTime = xTaskGetTickCount();

    /* Enable HSI if not already running */
    health_enable(true);
    do {
        if(!err) {
            err = cmd_magnet_current_check();
        }
        if(!err) {
            ms_since_since_last_info += COND_MONITOR_WAIT_MS;
            if(ms_since_since_last_info >= 1000) {
                ms_since_since_last_info = 0;
                TraceInfo(TrcMsgSeq, "magnet r:%d",
                        (monitor_loops - nloops)/(COND_MONITOR_LOOPS_SEC),
                        0,0,0,0,0);
            }
        }
        if(condition_step_status == CONDITION_RUN_STAT_ABORTED) {
            err = __LINE__;
            TraceInfo(TrcMsgSeq, "Abort during magnet monitor", 0,0,0,0,0,0);
        }
        /* Do not save here if there was an error, exit and stop ASAP */
        if(!err && (nloops % COND_STATUS_SAVE_LOOPS == 0)) {
            ctrl_condition_status_save();
        }
        if(!err) {
            /* Place this task in the blocked state until it is time to run again. */
            vTaskDelayUntil(&xNextWakeTime, (COND_MONITOR_WAIT_MS/portTICK_PERIOD_MS));
            stat->elapsed_ms += COND_MONITOR_WAIT_MS;
        }
        
    } while( ++nloops < monitor_loops && !err);
    
    return err;
}

static int ctrl_condition_monitor_keeper(uint32_t monitor_ms, condition_stat_t *stat)
{
    int err = 0;
    int keeper_is_running = 0;
    int keeper_hsi_updated = 0;
    
    float keeper_v = 0;
    
    uint32_t monitor_loops = monitor_ms/COND_MONITOR_WAIT_MS; /* how many loops (roughly) */
    uint32_t nloops = 0; /* loop counter */
    
    // Initialize xNextWakeTime - this only needs to be done once.
    TickType_t xNextWakeTime = xTaskGetTickCount();
    
    /* Enable HSI if not already running */
    health_enable(true);

    uint32_t ms_since_since_last_info = 0;

    do {
        /* Place this task in the blocked state until it is time to run again. */
        vTaskDelayUntil(&xNextWakeTime, (COND_MONITOR_WAIT_MS/portTICK_PERIOD_MS));
        stat->elapsed_ms += COND_MONITOR_WAIT_MS;
        
        keeper_is_running = ctrl_keeper_isrunning();
        if(!keeper_is_running) {
            TraceE2(TrcMsgSeq, "Keeper stopped running", 0,0,0,0,0,0);
            err = __LINE__;
        }
        if(!err) {
            keeper_hsi_updated = health_device_hsi_recieved(COMPONENT_KEEPER);
            if(!keeper_hsi_updated) {
                err = __LINE__;
            }
        }
        if(!err) {
            keeper_v = ctrl_keeper_v_out_get();
            if(keeper_v > CONDITIONING_KEEPER_VOLTAGE_LIMIT) {
                err = __LINE__;
                TraceE2(TrcMsgSeq, "Keeper voltage limit exceeded:%d",
                        (int)(keeper_v),0,0,0,0,0);
            }
        }
        if(!err) {
            err = cmd_magnet_current_check();
        }
        ms_since_since_last_info += COND_MONITOR_WAIT_MS;
        if(err || ms_since_since_last_info >= 1000) {
            ms_since_since_last_info = 0;
            TraceInfo(TrcMsgSeq, "keeper running:%d v:%d r:%d",
                    keeper_is_running, (int)(keeper_v * 1000),
                    (monitor_loops - nloops)/(1000/COND_MONITOR_WAIT_MS),0,0,0);
        }
        if(condition_step_status == CONDITION_RUN_STAT_ABORTED) {
            err = __LINE__;
            TraceInfo(TrcMsgSeq, "Abort during keeper monitor", 0,0,0,0,0,0);
        }
        /* Do not save here if there was an error, exit and stop ASAP */
        if(!err && (nloops % COND_STATUS_SAVE_LOOPS == 0)) {
            ctrl_condition_status_save();
        }
        
    } while((++nloops < monitor_loops) && !err);
    
    return err;
}

/**
 * Throttle up/down based on the power number provided and threshold constants
 * @param anode_p anode power
 * @param direction true = up, false = down
 * @return 0 on success, non-zero otherwise.   If power is low and it cannot
 * throttle down anymore, this will not fail.
 */
static int ctrl_condition_anode_throttle(bool direction)
{
    int err = 0;
    int current_setpoint = ctrl_sequence_setpoint_get();
    
    (void)current_setpoint; // Only used in trace
    
    TraceInfo(TrcMsgSeq, "Throttle %s. cs:%d",
            (int)(direction ? "up":"down"), current_setpoint + 1, 0,0,0,0);
    
    /* direction -- true = increment, false = decrement */
    err = ctrl_throttle_increment(direction);
    
    return err;
}

/**
 * Monitor the Anode I/V/P for the specified time.  This will throttle the
 * up/down based anode power and the high/low threshold constants
 * @param monitor_ms milliseconds to monitor
 * @return 0 on success, non-zero otherwise
 */
static int ctrl_condition_monitor_anode(uint32_t monitor_ms, sequence_limits_t *limits, condition_stat_t *stat)
{
    int err = 0;
    int anode_is_running = 0;
    int anode_hsi_updated = 0;
    
    float anode_v = 0;
    float anode_i = 0;
    float anode_p = 0;
    
    float input_p = 0;
    
    uint32_t nloops = 0;
    uint32_t ms_since_since_last_info = 0;
    uint32_t monitor_loops = monitor_ms/COND_MONITOR_WAIT_MS;
    
    /* loops to settle after throttling */
    uint32_t throttle_wait_countdown = COND_THROTTLE_SETTLE_CNT;
    
    /* Initialize xNextWakeTime - this only needs to be done once. */
    TickType_t xNextWakeTime = xTaskGetTickCount();
    
    /* Enable HSI if not already running */
    health_enable(true);

    do {
        /* Wait first, make sure at least ONE HSI message has had time to
         * arrived after HSI was enabled above */
        vTaskDelayUntil(&xNextWakeTime, (COND_MONITOR_WAIT_MS/portTICK_PERIOD_MS));
        stat->elapsed_ms += COND_MONITOR_WAIT_MS;
        ms_since_since_last_info += COND_MONITOR_WAIT_MS;
        
        anode_is_running = ctrl_anode_isrunning();
        if(!anode_is_running) {
            TraceE2(TrcMsgSeq, "Anode stopped running", 0,0,0,0,0,0);
            err = __LINE__;
        }
        if(!err) {
            anode_hsi_updated = health_device_hsi_recieved(COMPONENT_ANODE);
            if(!anode_hsi_updated) {
                err = __LINE__;
            }
        }
        if(!err) {
            /* Check Voltage, Current, and Power.  Only fail if system output
             * power fails.  Getting the values from the control module takes
             * very little time.  The values are updated continously in another
             * task, so getting the latest value here does not actually take the
             * time it would take to go out and query the device.  Plus, we want
             * to log ANYTHING that exceeded the limit.  However, the values
             * from the MCU are not updated often enough for the system control
             * processor to do any averaging.   On the real thruster, current
             * can be very noisy, so trying to limit and throttle here is
             * futile.  All judgments are made on overall system current, the
             * MCU(s) can protect themselves, they have the real-time values.
             */
            anode_v = ctrl_anode_v_out_get();
            if(anode_v > limits->voltage_limit) {
                TraceInfo(TrcMsgSeq, "Anode voltage limit exceeded:%d",
                        (int)(anode_v),0,0,0,0,0);
            }
            anode_i = ctrl_anode_i_out_get();
            if((anode_i) > limits->current_limit) {
                TraceInfo(TrcMsgSeq, "Anode current limit exceeded:%d",
                        (int)(anode_i),0,0,0,0,0);
            }
            anode_p = ctrl_anode_p_out_get();
            if((anode_p) > limits->power_limit) {
                err = __LINE__;
                TraceInfo(TrcMsgSeq, "Anode power maximum exceeded. v:%d i:%d p:%d l:%d",
                        (int)(anode_v),
                        (int)(anode_i),
                        (int)(anode_p), limits->power_limit,0,0);
            }
//            input_p = sys_adc_hsi_get_input_power();
            if(input_p > limits->max_limit) {
                TraceE2(TrcMsgSeq, "Input power limit exceeded:%d",
                       (int)(input_p),0,0,0,0,0);
            }
        }
        if(!err) {
            err = cmd_magnet_current_check();
        }
        /* Print info before checking throttling.  It makes it easer to see
         * what is happening -- print if we get an error too */
        if(err || ms_since_since_last_info >= 1000) {
            ms_since_since_last_info = 0;
            TraceInfo(TrcMsgSeq, "Anode tw:%d v:%d i:%d p:%d ip:%d r:%d",
                    throttle_wait_countdown,
                    (int)(anode_v), (int)(anode_i * 1000),
                    (int)(anode_p), (int)(input_p), 
                    (monitor_loops - nloops)/(COND_MONITOR_LOOPS_SEC));
        }
        if(!err) {
            /* Either wait for it to settle or check if it needs to 
             * throttle to a lower/higher setpoint */
            if(throttle_wait_countdown) {
                throttle_wait_countdown--;
                
            } else if(anode_p > limits->adjust_limit_upper ||
                      anode_p < limits->adjust_limit_lower) {
                throttle_wait_countdown = COND_THROTTLE_SETTLE_CNT;
                /* Do not fail here, just let it exceed the max or shutdown */
                ctrl_condition_anode_throttle(anode_p < limits->adjust_limit_lower);
            }
        }
        /* Do not save here if there was an error, exit and stop ASAP */
        if(!err && (nloops % COND_STATUS_SAVE_LOOPS == 0)) {
            ctrl_condition_status_save();
        }
        
        if(condition_step_status == CONDITION_RUN_STAT_ABORTED) {
            err = __LINE__;
            TraceInfo(TrcMsgSeq, "Abort during anode monitor", 0,0,0,0,0,0);
        }
        
    } while((++nloops < monitor_loops) && !err);

    return err;
}

static int ctrl_condition_run(sequence_condition_t *seq)
{
    int err = 0;
    uint32_t seq_run_stat = 0; /* Just the result of the seq run */
    sequence_run_t seq_run;
    
    if(pseq_semaphore == NULL) {
        /* initialize the semaphore if it hasn't already happened*/
        seq_semaphore = xSemaphoreCreateBinaryStatic(&seq_semaphore_buffer);
        pseq_semaphore = &seq_semaphore;
    }
    
    memset(&seq_run, 0, sizeof(seq_run));
    seq_run.seq = seq->seq;
    seq_run.size = seq->size;
    seq_run.name = seq->name;
    seq_run.status = &seq->stat->seq_stat_cond;
    /* add the semaphore so we can wait for the sequence to finish */
    seq_run.psem = pseq_semaphore;
    
    err = ctrl_sequence_run(&seq_run);
    if(!err) {
        /* Wait forever, for now. If the sequence never finishes, we have bigger
         * problems */
        err = xSemaphoreTake(*pseq_semaphore, portMAX_DELAY);
        seq_run_stat = SEQUENCE_STATUS_GET(seq->stat->seq_stat_cond);
        if(seq_run_stat != (uint32_t) SEQ_STAT_SUCCESS) {
            err = __LINE__;  /* Nothing more to do */
        } else {
            err = 0;
        }
    }
    if(!err) {
        /*
         * Clear any latched sequence errors
         */
        ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_CONDITIONING_FAULT);
        ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_THROTTLING_FAULT);
        
        uint32_t monitor_ms = seq->seq_ucv.monitor_ms;
        TraceDbg(TrcMsgSeq, "ms monitor:%d elapsed:%d",
                monitor_ms, seq->stat->elapsed_ms, 0,0,0,0);
        if(monitor_ms >  (seq->stat->elapsed_ms)) {
           monitor_ms -= (seq->stat->elapsed_ms);
        }


        /* This is not a continuation reset elapsed time*/
        if(seq->stat->elapsed_ms >= monitor_ms){
            /* Reset elapsed time and err */
            seq->stat->elapsed_ms = 0;
            seq->stat->monitor_err = 0;
        }
        switch(seq->type) {
            case CONDITION_MAGNET:
                err = ctrl_condition_monitor_magnets(monitor_ms, seq->stat);
                break;
            case CONDITION_KEEPER:
                err = ctrl_condition_monitor_keeper(monitor_ms, seq->stat);
                break;
            case CONDITION_ANODE:
            case CONDITION_ANODE_THROTTLE:
                err = ctrl_condition_monitor_anode(monitor_ms, &seq->seq_ucv.limits, seq->stat);
                break;
            default:
                err = __LINE__;
        }
        TraceDbg(TrcMsgSeq, "err:%d ms:%d elapsed:%d", err, monitor_ms, seq->stat->elapsed_ms,0,0,0);
        
        seq->stat->monitor_err = err;
        if(!err) {
            /* if the monitor routine passed, force it the exact time and save */
            seq->stat->elapsed_ms = seq->seq_ucv.monitor_ms;
            ctrl_condition_status_save();
        }
        
        if(err){
            client_control_specific_detail_t a = {0};
            a.err = err;
            ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_SQNC_CONDITIONING_FAULT, &a);
        } 
    }
    TraceDbg(TrcMsgSeq, "%s err:%d", (int)seq->name, err,0,0,0,0);
    return err;
}

uint32_t ctrl_condition_stat(void)
{
    return *pcurrent_cond_stat;
}

static void ctrl_condition_task(void *pvParameters)
{
    int err = 0;
    uint32_t step = condition_start_step;
    
    sequence_condition_t *scp = NULL;
    
#if COND_AUTO_RETRY
    int retries = 0;
#endif
    
    while(step < THROTTLE_COND_COUNT && !err) {
        condition_current_step = step + 1; /* For the outside world - index at 1 */
        condition_step_status = CONDITION_RUN_STAT_RUNNING;
        scp = &sequence_condition_sequences[step];
        scp->stat = &cnvm.condition_status[step];
        pcurrent_cond_stat = &cnvm.condition_status[step].seq_stat_cond;
        if(scp->type == CONDITION_ANODE_THROTTLE) {
            /* When the anode is already running we have to run a throttle seq */
            ctrl_sequence_setpoint_set(ANODE2_THROTTLE_SETPOINT);
            scp->seq = ctrl_throttle_seq_get(ANODE2_THROTTLE_SETPOINT);
            TraceDbg(TrcMsgSeq, "throttle point:%d name:%s", 
                    ANODE2_THROTTLE_SETPOINT, scp->name, 0,0,0,0);
        }
        
        err = ctrl_condition_run(scp);
        TraceInfo(TrcMsgSeq, "Condition Sequence %d %s: e:%d",
                step+1, (int)(err ? "failed" : "passed"), err,0,0,0);
        if(err) {
#if     COND_AUTO_RETRY
            if(condition_step_status != CONDITION_RUN_STAT_ABORTED &&
               step < COND_AUTO_RETRY_STEP_MAX && retries < COND_AUTO_RETRY_MAX) {
                retries++;
                /* Make sure it gets shutoff */
                ctrl_sequence_error_shutdown();
                condition_step_status = CONDITION_RUN_STAT_RETRY_DELAY;
                /* Make sure to save progress */
                ctrl_condition_status_save();
                TickType_t xNextWakeTime = xTaskGetTickCount();
                /* Place this task in the blocked state until it is time to try again. */
                vTaskDelayUntil(&xNextWakeTime, (COND_FAIL_RETRY_DELAY_MS/portTICK_PERIOD_MS));
                err = ctrl_condition_can_run(sequence_condition_sequences[step].required_state);
            }
            if(err) {
                condition_step_status = CONDITION_RUN_STAT_FAILED;
            }
#else
            condition_step_status = CONDITION_RUN_STAT_FAILED;
#endif
        } else {
            condition_step_status = CONDITION_RUN_STAT_PASSED;
            step++;
        }
    }
    /* Error or not, its over, shut everything off */
    ctrl_sequence_error_shutdown();
    /* One final save to be sure */
    ctrl_condition_status_save();
    client_set_conditioning_task_state(false);
    TraceInfo(TrcMsgSeq, "Condition task delete", 0,0,0,0,0,0);
    vTaskDelete(NULL);
}

int ctrl_condition_abort(void)
{
    int err = 0;
    if(client_get_conditioning_task_state()) {
        condition_step_status = CONDITION_RUN_STAT_ABORTED;
    }
    return err;
}

int ctrl_condition_start(uint32_t start_step)
{
    int err = 0;
    start_step -= 1;
    if(start_step >= THROTTLE_COND_COUNT) {
        err = __LINE__;
    }
    if(!err && !client_get_conditioning_task_state()) {
        ctrl_condition_status_read();
        condition_start_step = start_step;
        client_update_state(); // Make sure thruster state is up-to-date
        err = ctrl_condition_can_run(
                sequence_condition_sequences[condition_start_step].required_state);
        if(!err) {
            client_set_conditioning_task_state(true);
            cond_task_handle = xTaskCreateStatic(ctrl_condition_task, "Condition Task",
                    cond_TASK_STACK_SIZE, NULL, cond_TASK_PRIORITY,
                    cond_task_stack, &cond_TaskBuffer);
        }
    }
    return err;
}

int ctrl_condition_step_rw(unsigned int table, unsigned int step, 
        uint32_t* val, unsigned int which, unsigned int rw)
{
    int err = 0;
    uint32_t* p = NULL;
    
    if(table >= THROTTLE_COND_COUNT || step >= SEQUENCE_MAX_STEPS_CONDITION) {
        err = __LINE__;
    } else {
        p = (uint32_t*)&sequence_condition_sequences[table].seq[step];
    }
    
    if(!err){
        if(which == UPPER_32){
            if(rw == SEQ_WRITE){
                p[1] = *val;
            } else if(rw == SEQ_READ){
                *val = p[1];
            }
        } else if(which == LOWER_32){
             if(rw == SEQ_WRITE){
                p[0] = *val;
            } else if(rw == SEQ_READ){
                *val = p[0];
            }
        } else {
            err = __LINE__;
        }
    }
    
    return err;
}

int ctrl_condition_nvm_reset(void)
{
    condition_status_initialized = true;
    memset(cnvm.data, 0, sizeof(cnvm.data));
    return ctrl_condition_status_save();
}

int ctrl_condition_stats_get(condition_steps_stat_t *stats)
{
    int err = 0;
    /* read will succeed or zero it out, so don't bother with the return */
    ctrl_condition_status_read();
    stats->count = THROTTLE_COND_COUNT;
    stats->step         = &condition_current_step;
    stats->step_stats   = cnvm.condition_status;
    return err;
}
