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

#define MODULE_NUM MODULE_NUM_ICM
#define SUBMODULE_NUM TC_ICM_SUBMODULE

#include "definitions.h"
#include "utilities/trace/trace.h"
#include "error/error_handler.h"
#include "update/update_helper.h"
#include "tc_icm.h"
#include "iacm/iacm.h"
#include "mem_scrubber.h"
#include "hsi_memory.h"
#include "sys/timers/sys_timers.h"

#define DETAIL_LOG_LENGTH LOG_LENGTH_16

typedef struct tc_icm_error_detail{
    base_error_detail_t b_d;
    tc_icm_specific_error_detail_t tc_icm_specific_error_detail;
} tc_icm_error_detail_t;    

static tc_icm_error_detail_t error_detail[DETAIL_LOG_LENGTH];


static void* tc_icm_error_detail_strategy(void* arg);

#define ENABLE_DEBUG 0
#if ENABLE_DEBUG == 0
#ifdef  TraceDbg
#undef  TraceDbg
#define TraceDbg(m, fmt, d1,d2,d3,d4,d5,d6)
#endif
#endif

/* Define the ICM number of region used */
#define TC_ICM_NUM_REGION                 (4)
/* Define the ICM hash size in words */
#define TC_ICM_HASH_SIZE_WORD             (8)

/* Variable containing the bitfield of the ICM region were hash completed event occurs */
static volatile uint8_t region_hash_completed;

/* Variable containing the bitfield of the ICM region were digest mismatch event occurs */
static volatile uint8_t region_digest_mismatch;


/* Hash Buffer were ICM will store the computed hash for each region */
static volatile uint32_t __attribute__((aligned (128))) buffer_hash[TC_ICM_NUM_REGION][TC_ICM_HASH_SIZE_WORD] = {0};

static efc_hsi_t     *efc_hsi = &hsi_mem.efc_hsi;
static sys_mem_hsi_t *sys_mem_hsi = &hsi_mem.sys_mem;


#define ICM_MISMATCH_MASK (ICM_INT_MSK_DIGEST_MISMATCH_R0_MASK | \
                           ICM_INT_MSK_DIGEST_MISMATCH_R1_MASK | \
                           ICM_INT_MSK_DIGEST_MISMATCH_R2_MASK | \
                           ICM_INT_MSK_DIGEST_MISMATCH_R3_MASK)

#define ICM_HASH_COMPLETE (ICM_REGION0_MASK | ICM_REGION1_MASK | ICM_REGION2_MASK | ICM_REGION3_MASK)

#define ICM_APPLICATION_REGION_MASK ICM_INT_MSK_DIGEST_MISMATCH_R1_MASK

/* Flag to for repair retry */
#define REPAIR_RETRY_FLAG 0x725f7276  /* 'vr_r' (verify and repair retry) */
static uint32_t repair_failure = 0;

#define REPAIR_ATTEMPTS_MAX 3
static int repair_retries = 0;

/* How long to wait after a failed repair attempt or ICM mismatch */ 
#define REPAIR_SECONDS (5 SECONDS)
static SYSTMR repair_timer = 0;

/* Block timeout for the ICM/Memory Scrubber */
#define ICM_MUTEX_BLOCK_MS 5000

static OSAL_MUTEX_HANDLE_TYPE icm_lock;
/* For a lazy init */
static OSAL_MUTEX_HANDLE_TYPE *p_icm_lock = NULL;

static void tc_icm_start(void);

/**
 * Stops and resets the ICM. MUST be called prior to any FLASH memory modifications.
 *
 */
static void tc_icm_stop(void)
{
    TraceDbg(TrcMsgMemSrb, "stop", 0,0,0,0,0,0);
    ICM_Disable();          // disable the ICM during repairs
    ICM_SoftwareReset();    // and also reset it;
}

/**
 * Integrity Check Monitor ISR callback function
 * 
 * @param context pointer to context callback
 * 
 */
static void tc_icm_callback(uintptr_t context) 
{
    // get the ICM's interrupt ID and save it to IACM memory
    ICM_INT_MSK status = (ICM_InterruptGet() & ICM_InterruptMasked());
    TraceDbg(TrcMsgMemSrb, "status:0x%d", status,0,0,0,0,0);
    
    if(status & ICM_MISMATCH_MASK)  {
        iacm_set_from_isr(IACM_ICM_INT_ID, status);
        ICM_Disable();
        ICM_SoftwareReset();
        region_digest_mismatch = (status & ICM_MISMATCH_MASK);
        if (status & ICM_APPLICATION_REGION_MASK) {
            uh_do_reset();      // reset back to the bootloader for repair ASAP
        }
    }
    /* Check Hash Complete interrupts */
    if (status & ICM_INT_MSK_HASH_R0_MASK) {
        region_hash_completed |= ICM_REGION0_MASK;
        ICM_InterruptDisable(ICM_INT_MSK_HASH_R0_MASK);
    }
    if (status & ICM_INT_MSK_HASH_R1_MASK) {
        region_hash_completed |= ICM_REGION1_MASK;
        ICM_InterruptDisable(ICM_INT_MSK_HASH_R1_MASK);
    }
    if (status & ICM_INT_MSK_HASH_R2_MASK) {
        region_hash_completed |= ICM_REGION2_MASK;
        ICM_InterruptDisable(ICM_INT_MSK_HASH_R2_MASK);
    }
    if (status & ICM_INT_MSK_HASH_R3_MASK) {
        region_hash_completed |= ICM_REGION3_MASK;
        ICM_InterruptDisable(ICM_INT_MSK_HASH_R3_MASK);
    }
}



static void tc_icm_start_retry_timer(void)
{
    sys_timer_start(REPAIR_SECONDS, &repair_timer);
    repair_failure = REPAIR_RETRY_FLAG;
    repair_retries++;
}

/**
 * Verifies memory regions, repairs them if it can, enables the ICM, then does
 * the verify again.  IF the second verify fails, another round of verify,
 * repair, enable ICM, verify again is schedule for later.  It does this until
 * the max retries is exceeded, or it passes.
 */
void tc_icm_verify_repair_enable(void)
{
    uint32_t err = 0;
    
    err = ms_verify_repair();
    TraceDbg(TrcMsgMemSrb, "region stat:0x%08x", err, 0,0,0,0,0);
    
    tc_icm_start();
    
    err = ms_verify();
#ifndef PRODUCTION_BUILD
    // Ignore Boot Loader and App Executables
    err &= ~(IACM_BOOTLOADER_EXE | IACM_SYSTEM_CONTROL_EXE);
#endif
    if(!err) {
        ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_ECP_MEMORY_FAULT);
        repair_failure = 0;
        repair_retries = 0;
    } else {
        // Try again later
        tc_icm_start_retry_timer();
        ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_ECP_MEMORY_FAULT, NULL);
    }
}



/*
 * Get size of whole error detail struct
 */
size_t tc_icm_error_detail_size_get(void)
{
    return sizeof(tc_icm_specific_error_detail_t);
}


static void* tc_icm_error_detail_strategy(void* arg)
{
    if(arg != NULL){
        EH_LOG_ERROR(tc_icm_specific_error_detail, arg);
    }
    
    
    return eh_get(MODULE_NUM, SUBMODULE_NUM)->error_log;
}

/**
 * Service the ICM tasks.
 */
void tc_icm_serivce(void)
{
    
    if(region_digest_mismatch) {
        tc_icm_specific_error_detail_t d = {.digest = region_digest_mismatch};
        ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_ECP_MEMORY_FAULT, &d);
        TraceE2(TrcMsgErr2, "Memory Digest error in region:0x%x",
                region_digest_mismatch,0,0,0,0,0);
        int lock_err = tc_icm_lock();
        if(!lock_err) {
            tc_icm_verify_repair_enable();
            tc_icm_unlock();
        } else {
            tc_icm_start_retry_timer();
        }
    }
    if(repair_failure == REPAIR_RETRY_FLAG && repair_timer == 0) {
        if(repair_retries == (REPAIR_ATTEMPTS_MAX + 1)) {
            TraceE2(TrcMsgMemSrb, "Max Repair Attempts:%d exceeded. ms:0x%08x",
                    repair_retries-1, iacm_get(IACM_REPAIR_STATUS), 0,0,0,0);
            repair_retries++; /* say something once, why say it again */
        } else if(repair_retries <= REPAIR_ATTEMPTS_MAX) {
            int lock_err = tc_icm_lock();
            if(!lock_err) {
                TraceInfo(TrcMsgMemSrb, "Repair retry attempt:%d", repair_retries, 0,0,0,0,0);
                tc_icm_stop();
                tc_icm_verify_repair_enable();
                tc_icm_unlock();
            }
        }
    }
    uint32_t err = EFC_REGS->EEFC_FSR;
    if(err & EFC_ECC_ERROR) {
        tc_icm_specific_error_detail_t d = {.eefc_fsr = err};
        ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_ECP_EEFC_EEC_ERROR, &d);
        
        if(err & EEFC_FSR_MECCEMSB_Msk) efc_hsi->count_meccemsb++;
        if(err & EEFC_FSR_UECCEMSB_Msk) efc_hsi->count_ueccemsb++;
        if(err & EEFC_FSR_MECCELSB_Msk) efc_hsi->count_meccelsb++;
        if(err & EEFC_FSR_UECCELSB_Msk) efc_hsi->count_ueccelsb++;
        TraceE2(TrcMsgErr2, "ECC err:%x MMSB:%d UMSB:%d MLSB:%d ULSB:%d", err,
                efc_hsi->count_meccemsb,
                efc_hsi->count_ueccemsb,
                efc_hsi->count_meccelsb,
                efc_hsi->count_ueccelsb, 0);
    }
    sys_mem_hsi->failed_repairs = iacm_get(IACM_FAILED_REPAIRS);
    sys_mem_hsi->region_stat    = iacm_get(IACM_MEM_REGION_STATUS);
    sys_mem_hsi->repair_stat    = iacm_get(IACM_REPAIR_STATUS);
}

/**
 * Initialize the TCM Module
 */
static void tc_icm_init(void)
{
    TraceDbg(TrcMsgMemSrb, "icm init", 0,0,0,0,0,0);
    memset(efc_hsi, 0, sizeof(efc_hsi_t));
    
    ICM_SetHashStartAddress((uint32_t)&buffer_hash);
    
    region_hash_completed = 0;
    region_digest_mismatch = 0;
    
    ICM_CallbackRegister(tc_icm_callback, (uintptr_t)NULL);

    ICM_InterruptEnable(ICM_INT_MSK_HASH_R0_MASK | ICM_INT_MSK_HASH_R1_MASK |
                        ICM_INT_MSK_HASH_R2_MASK | ICM_INT_MSK_HASH_R3_MASK);
    
    ICM_Enable();
    
    while (region_hash_completed != ICM_HASH_COMPLETE);
    TraceDbg(TrcMsgMemSrb, "hash complete:0x%x:0x%x", 
            region_hash_completed, ICM_HASH_COMPLETE,0,0,0,0);
    
    ICM_Disable();
    ICM_WriteBackDisable(true);
    
    ICM_MonitorEnable(ICM_REGION0_MASK | ICM_REGION1_MASK | ICM_REGION2_MASK | ICM_REGION3_MASK);
    ICM_InterruptEnable(ICM_INT_MSK_DIGEST_MISMATCH_R0_MASK | ICM_INT_MSK_STATUS_UPDATE_R0_MASK |
                        ICM_INT_MSK_DIGEST_MISMATCH_R1_MASK | ICM_INT_MSK_STATUS_UPDATE_R1_MASK |
                        ICM_INT_MSK_DIGEST_MISMATCH_R2_MASK | ICM_INT_MSK_STATUS_UPDATE_R2_MASK | 
                        ICM_INT_MSK_DIGEST_MISMATCH_R3_MASK | ICM_INT_MSK_STATUS_UPDATE_R3_MASK);
    ICM_Enable();
    TraceDbg(TrcMsgMemSrb, "icm init done", 0,0,0,0,0,0);
}


void tc_icm_error_handler_init(void)
{
    eh_create(MODULE_NUM, SUBMODULE_NUM, tc_icm_error_detail_strategy, DETAIL_LOG_LENGTH, error_detail);
    fh_fault_handlers_register(ERROR_CODE_ECP_MEMORY_FAULT, FH_ALERT);
    fh_fault_handlers_register(ERROR_CODE_ECP_EEFC_EEC_ERROR, FH_ALERT);
}

/**
 * Stops and resets the ICM. MUST be called prior to any FLASH memory modifications.
 * Also, stops any retries
 */
void tc_icm_disable(void)
{
    TraceDbg(TrcMsgMemSrb, "disable", 0,0,0,0,0,0);
    tc_icm_stop();
    /* Stop retry until it is enable again */
    repair_failure = 0;
    repair_retries = 0;
}


/**
 * Initializes and starts the ICM memory monitor process. First the low level
 * init then the application inits and kick off.
 *
 */
static void tc_icm_start(void)
{
    TraceDbg(TrcMsgMemSrb, "start", 0,0,0,0,0,0);
    ICM_Initialize();       // low level initialization first
    tc_icm_init();          // application initializations
}

/**
 * Lock the ICM and memory scrubber to prevent anything from modifying the
 * memory while the memory scrubber is verifying and repairing, and to prevent
 * anyone from enabling/disabling the ICM while all that is happening
 * 
 * @return 0 upon if lock was obtained, non-zero otherwise
 */
int tc_icm_lock(void)
{
    /* Allow for watchdog petting while waiting on the lock */
    int wait_loops = ICM_MUTEX_BLOCK_MS/1000;
    int loop = 0;
    
    OSAL_RESULT err = OSAL_RESULT_FALSE;
    do {
        err = OSAL_MUTEX_Lock(p_icm_lock, 1000);
        WATCHDOG_CLEAR();
        loop++;
    } while(loop < wait_loops && err == OSAL_RESULT_FALSE);
    
    return err == OSAL_RESULT_TRUE ? 0:__LINE__;
}
/**
 * Unlock the memory scrubber mutex
 */
void tc_icm_unlock(void)
{
    if(p_icm_lock) {
        OSAL_MUTEX_Unlock(p_icm_lock);
    }
}

void tc_icm_lock_init(void)
{
    if(p_icm_lock == NULL) {
        static StaticSemaphore_t xMutexBuffer;
        icm_lock = xSemaphoreCreateMutexStatic(&xMutexBuffer);
        p_icm_lock = &icm_lock;
    }
}