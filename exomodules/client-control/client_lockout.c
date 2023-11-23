#include <memory.h>
#include "client_lockout.h"
#include "checksum.h"
#include "client_service.h"
#include "client_p.h"
#include "storage/storage_memory_interface.h"
#include "sys/sys_timers.h"
#include "trace/trace.h"
#include "error/error_handler.h"
#include "thruster_control.h"
#include "user-setting-values/client_control_usv.h"

#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_LOCKOUT

typedef struct _lockout_nvm {
  int32_t timer;
  int32_t reason;
  uint16_t crc;
} lockout_nmv_t;

#define CLIENT_LOCKOUT_TIME (1 MINUTES)

static lockout_nmv_t client_lockout_nvm;
static SYSTMR client_lockout_timer;
static SYSTMR client_lockout_next_save;

/**
 * Take the provided time, calc the next save time, then make sure a negative
 * number gets set to zero.  The timer is unsigned, so a negative result would
 * end up being very large.
 * @param current_to current time out.
 * @return next save time or zero if next save is shorter than the current time.
 */
static uint32_t client_calc_next_lo_save(int32_t current_to)
{
    int32_t next_save = 0;
    int32_t save_interval = cc_usv_lockout_save_time_get();

    next_save = current_to - save_interval;
    if(next_save < 0) {
        next_save = 0;
    }
    return (uint32_t) next_save;
}

static int client_lockout_nvm_write(lockout_nmv_t *lo)
{
    int err = 0;
    int fd = -1;
    char *key = LOCKOUT_TIMER;

    uint8_t *p = (uint8_t*) lo; // For calculating crc

    lo->crc = 0;
    for(size_t i = 0; i < (sizeof(lockout_nmv_t) - 2); i++){
        lo->crc = update_crc_16(lo->crc, p[i]);
    }
    TraceInfo(TrcMsgMcuCtl, "nvm crc:%x r:%x t:%x",
              lo->crc, lo->reason, lo->timer, 0,0,0);

    err = storage_memory_open(key, O_WRONLY);
    if(err >= 0){
        fd = err;
        err = 0;
    } else {
        err = __LINE__;
    }
    if(!err) {
        err = storage_memory_write(fd, (uint8_t*)lo, sizeof(lockout_nmv_t));
        if(err != sizeof(lockout_nmv_t)) {
            err = __LINE__;
        } else {
            err = 0;
        }
    }
    if(fd >= 0) {
        storage_memory_close(fd);
    }
    return err;
}


/**
 * Lockout error handler, the fault handler queues this to the client control
 * task when a lockout is detected
 * @param arg reason (uint32_t *)
 * @return 0
 */
static int client_error_lockout(void *arg)
{
    client_lockout_nvm.reason = *((int*) arg);
    TraceE2(TrcMsgErr2, "Lockout error:%x nvm_t:%x t:%x",
            client_lockout_nvm.reason, 
            client_lockout_nvm.timer,
            client_lockout_timer,0,0,0);
    
    ctrl_sequence_shutdown(NULL);
    
    if(!client_lockout_nvm.timer) {
        /* TImer not set from NVM on power up, get the full time */
        client_lockout_nvm.timer = cc_usv_lockout_time_get();
    }
    if(client_lockout_timer > 0) {
        sys_timer_abort(&client_lockout_timer);
    }
    client_lockout_timer = client_lockout_nvm.timer;
    sys_timer_start(client_lockout_timer, &client_lockout_timer);
    client_lockout_next_save = client_calc_next_lo_save(client_lockout_timer);
    
    client_lockout_val_set(IACM_THRUSTER_STATE_LOCKOUT);
    client_lockout_reason_set(client_lockout_nvm.reason);
    client_lockout_nvm_write(&client_lockout_nvm);
    
    return 0;
}

uint32_t client_lockout_val_get(void)
{
//    uint32_t lockout_val = iacm_get(IACM_THRUSTER_LOCKOUT_VAL);
//    return lockout_val;
return 0;
}

void client_lockout_val_set(iacm_thruster_state_t iacm_lockout_state)
{
//    iacm_set(IACM_THRUSTER_LOCKOUT_VAL, iacm_lockout_state);
}

uint32_t client_lockout_reason_get(void)
{
//    uint32_t lockout_reason = iacm_get(IACM_LAST_LOCKOUT_REASON);

    return 0;
}

void client_lockout_reason_set(uint32_t reason)
{
//    iacm_set(IACM_LAST_LOCKOUT_REASON, reason);
}

static int client_lockout_timer_val_get(void)
{
//    int lockout_timer_val = iacm_get(IACM_THRUSTER_LOCKOUT_TIMER);
    return 0;
}

static void client_lockout_timer_set(uint32_t timer)
{
//    iacm_set(IACM_THRUSTER_LOCKOUT_TIMER, timer);
}

static int client_lockout_nvm_read(lockout_nmv_t *lo)
{
    int err = 0;
    int fd = -1;
    char *key = LOCKOUT_TIMER;
    
    uint8_t *p = (uint8_t*) lo; // For calculating crc
    uint16_t match_crc = 0;
    
    err = storage_memory_open(key, O_RDONLY);
    if(err >= 0){ 
        fd = err;
        err = 0;
    } else {
        err = __LINE__;
    }
    if(!err) {
        err = storage_memory_read(fd, (uint8_t*)lo, sizeof(lockout_nmv_t));
        if(err != sizeof(lockout_nmv_t)) {
            err = __LINE__;
        } else {
            err = 0;
        }
    }
    if(fd >= 0) {
        storage_memory_close(fd);
    }
    
    if(!err) {
        for(size_t i = 0; i < (sizeof(lockout_nmv_t) - 2); i++){
            match_crc = update_crc_16(match_crc, p[i]);
        }
        TraceInfo(TrcMsgMcuCtl, "nvm crc:%x mcrc:%x r:%x t:%x",
            lo->crc, match_crc, lo->reason, lo->timer,0,0);
        if(match_crc != lo->crc) {
            err = __LINE__;
            memset(lo, 0, sizeof(lockout_nmv_t));
        }
    }
    return err;
}

void client_lockout_init(void)
{
    bool is_locked_out = (client_lockout_val_get() == IACM_THRUSTER_STATE_LOCKOUT);
    if(is_locked_out) {
        client_lockout_timer = client_lockout_timer_val_get();
        client_lockout_nvm.timer = client_lockout_timer;
    } else {
        // Check NVM
        TraceInfo(TrcMsgMcuCtl, "check nvm", 0,0,0,0,0,0);
        int err = client_lockout_nvm_read(&client_lockout_nvm);
        TraceInfo(TrcMsgMcuCtl, "check nvm err:%d r:%x to:%x crc:%x", err,
                client_lockout_nvm.reason,
                client_lockout_nvm.timer,
                client_lockout_nvm.crc,0,0);
        
        if(!err && client_lockout_nvm.timer > 0) {
            is_locked_out = true;
            client_lockout_val_set(IACM_THRUSTER_STATE_LOCKOUT);
            client_lockout_reason_set(client_lockout_nvm.reason);
            client_lockout_timer_set(client_lockout_nvm.timer);
            client_lockout_timer = client_lockout_nvm.timer;
        }
    }
    TraceInfo(TrcMsgMcuCtl, "is lo:%d", is_locked_out,0,0,0,0,0);
    if(is_locked_out) {
        client_lockout_next_save = client_calc_next_lo_save(client_lockout_timer);
        TraceInfo(TrcMsgMcuCtl, "set lockout timer:%x next save:%x",
                client_lockout_timer, client_lockout_next_save, 0,0,0,0);
        sys_timer_start(client_lockout_timer, &client_lockout_timer);
    } else {
        TraceInfo(TrcMsgMcuCtl, "thruster safe", 0,0,0,0,0,0);
        client_lockout_val_set(IACM_THRUSTER_STATE_SAFE);
    }
}

void client_lockout_expired(void)
{
    client_lockout_status_set(LOCKOUT_ERROR_CLEAR);
    client_lockout_val_set(IACM_THRUSTER_STATE_SAFE);
    client_lockout_timer_set(0);
    client_lockout_nvm.timer = 0;
    client_lockout_nvm_write(&client_lockout_nvm);
}

/**
 * Because the lockout time is just too long in development
 */
void client_lockout_override(void)
{
    if(client_lockout_timer > 0) {
        sys_timer_abort(&client_lockout_timer);
    }
}

/**
 * Queue the lockout to be handled by client control.
 * 
 * Note: If the client queue is full, the lockout will be performed here
 * 
 * @param reason reason for lockout
 */
void client_lockout_queue(int reason)
{
    client_service_t service = {0};
    static int lockout_reason = 0;
    
    lockout_reason = reason;
    service.params = &lockout_reason;
    service.cb = client_error_lockout;
    
    TraceInfo(TrcMsgMcuCtl, "", 0,0,0,0,0,0);
    
    BaseType_t xStatus = client_service_queue(&service);
    if(xStatus != true) {
        // Queue full, do it now.
        client_error_lockout(&reason);
    } else {
        TraceE2(TrcMsgErr2, "Lockout error queued:%d", reason,0,0,0,0,0);
    }
}

/* 
 * Check lockout status after client error handling and canopen emcy node init
 *   so we can set lockout errors.  
 */
void client_lockout_status_set(lockout_action_t lockout_action)
{
    uint32_t lockout_reason = client_lockout_reason_get();
    uint32_t lockout_status = client_lockout_val_get();
    
    /*
     * Check if the lockout reason is a legitimate error code, if not set it as an unknown
     * error code
     * 
     */
    if(lockout_reason >= ERROR_CODE_EOL){
        lockout_reason = ERROR_CODE_UNKNOWN_ERR;
    }
    
    /*
     * Check persistent memory to see if we need to set/clear lockout error bits 
     */
    if(lockout_status == IACM_THRUSTER_STATE_LOCKOUT && 
       lockout_action == LOCKOUT_ERROR_SET){
        ERROR_SET(TC_EMCY_REG_CURRENT, lockout_reason, NULL);
    }else {
        ERROR_CLEAR(TC_EMCY_REG_CURRENT, lockout_reason);
    }   
}

inline uint32_t client_lockout_timer_get(void)
{
  return client_lockout_timer;
}

void client_lockout_update_timer(void)
{
  /* update time in memory */
  client_lockout_timer_set(client_lockout_timer);
  if(client_lockout_timer < client_lockout_next_save) {
      TraceInfo(TrcMsgAlways, "to:%x s:%x",
              client_lockout_timer, client_lockout_next_save,0,0,0,0);
      /* not the original & its time save */
      client_lockout_nvm.timer = client_lockout_timer;
      client_lockout_nvm_write(&client_lockout_nvm);
      client_lockout_next_save = client_calc_next_lo_save(client_lockout_timer);
      TraceInfo(TrcMsgAlways, "to:%x s:%x",
              client_lockout_timer, client_lockout_next_save,0,0,0,0);
  }
}