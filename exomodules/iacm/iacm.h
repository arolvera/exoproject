/**
 * @file    iacm.h
 *
 * @brief   ??? Is this the ATMEGA Integrity Check Monitor? What is the purpose of this module?
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

#ifndef LIB_IACM_H
#define	LIB_IACM_H

#ifdef	__cplusplus
extern "C" {
#endif
    
    
#ifdef IACM_OSAL_MUTEX 
#define LOCK_SCHEME 1
#endif
    
#ifdef IACM_NO_MUTEX
#define LOCK_SCHEME 2
#endif

#define IACM_STATE_UPDATING         = 0x80000000,
#define IACM_STATE_ERROR            = 0xffffffff,

#define IACM_SYSTEM_UPDATE_POS      (31)
#define IACM_SYSTEM_UPDATE          (1 << IACM_SYSTEM_UPDATE_POS)

#define IACM_SYSTEM_CONTROL_EXE_POS (25)
#define IACM_SYSTEM_CONTROL_EXE     (1 << IACM_SYSTEM_CONTROL_EXE_POS)

#define IACM_BOOTLOADER_EXE_POS     (24)
#define IACM_BOOTLOADER_EXE         (1 << IACM_BOOTLOADER_EXE_POS)

#define IACM_UPDATE_VERIFY_POS      (16)
#define IACM_UPDATE_VERIFY_MASK     (0xFFFFUL << IACM_UPDATE_VERIFY_POS)
#define IACM_UPDATE_VERFIY(_v_)     (IACM_UPDATE_VERIFY_MASK & (_v_ << IACM_UPDATE_VERIFY_POS))

#define IACM_UPDATE_INSTALL_POS     (0)
#define IACM_UPDATE_INSTALL_MASK    (0xFFFFUL << IACM_UPDATE_INSTALL_POS)
#define IACM_UPDATE_INSTALL(_v_)    (IACM_UPDATE_INSTALL_MASK & (_v_ << IACM_UPDATE_INSTALL_POS))

#define IACM_UPDATE_APP_POS         (8)
#define IACM_UPDATE_APP_MASK        (1 << IACM_UPDATE_APP_POS)
#define IACM_UPDATE_BL_POS          (9)
#define IACM_UPDATE_BL_MASK         (1 << IACM_UPDATE_BL_POS)

/* Tells APP the bootloader hit an error */
#define IACM_UPDATE_ERR_BL_FLAG     (1 << 31)

// IACM thruster state values
typedef enum _iacm_thruster_state
{
    IACM_THRUSTER_STATE_UNINITIALIZED = 0,
    IACM_THRUSTER_STATE_SAFE          = 0x65666173UL, /* SAFE in ascii */
    IACM_THRUSTER_STATE_LOCKOUT       = 0x6b636f6cUL, /* LOCK in ascii */
}   iacm_thruster_state_t;

typedef enum _iacm_fw_fault
{
    IACM_FWF_NO_FAULT = 0,
    IACM_FWF_UNDEFINED_IRQ,
    IACM_FWF_NMI,
    IACM_FWF_HARD_FAULT,
    IACM_FWF_DEBUG_MON,
    IACM_FWF_MEM_MANAGER,
    IACM_FWF_BUS_FAULT,
    IACM_FWF_USAGE_FAULT,    
}   IACM_FWF_t;

typedef enum exe_repair
{
    EXE_REPAIR_ALLOWED = 0,          /* Default to zero  */
    EXE_REPAIR_BLOCKED = 0x424C434B, /* BLCK in ASCII - Block Repairs */
} RESET_REPAIR_t;

// IACM Item enum. These values are used to index the IACM registers. No such
// array is declared but the IACM memory area is accessed as if it were an
// array through a pointer and the indexes enumerated below.
typedef enum _iacm_item
{
    IACM_UNUSED0 = 0,       
            
    IACM_MEM_REGION_STATUS, // ICM monitored memory region status bits, 0 = good component, 1 = corrupt
    IACM_FAILED_REPAIRS,    // components that failed to be updated, 0 = OK, 1 = failed repair
    IACM_REPAIR_STATUS,     // bits are same as IACM_MEM_REGION_STATUS, 1 = JUST REPAIRED
            
    IACM_UNUSED1,           
    IACM_THRUSTER_LOCKOUT_TIMER,    /* Lockout timer remaining                      */
    IACM_THRUSTER_LOCKOUT_VAL,      /* See iacm_thruster_state_t                    */
            
    IACM_INSTALL_STATUS,            /* Saved status of the FW updates/installs      */
                                    /* Format = 0xVVVVIIII  where:                  */
                                    /* V = verified components,                     */
                                    /* I = installed components                     */
                                    /* Verify and install bits are component numbers*/
                                    /*  - see thruster_control.h component_type_t   */
                                    /*    where the bits are shift 1 << componnet_t */
                                    /*  - App & Bootloader exe bits are above       */
    
    IACM_ICM_INT_ID,                /* ICM Interrupt ID, value indicator            */
    
    IACM_UPDATE_TRIGGER,            /* bootloader update trigger, value indicator   */
    
    IACM_TC_BL_VERSION,             /* bootloader version                           */
    IACM_TC_BL_GIT_SHA,             /* bootloader GIT commit number                 */

    IACM_EXECUTION_REPAIR,          /* Used during development to block the memory  */
                                    /* scrubber from repairing a programmed app     */
                                    /* or bootloader during development             */
                                    /* See RESET_REPAIR_t enum for values           */

    IACM_LAST_RESET_REASON,         /* RSTC_REGS->RSTC_SR read on boot              */
    IACM_FIRMWARE_FAULT,            /* Set by dummy and exception interrupt handlers*/

    IACM_INSTALL_ERROR,             /* Store install errors MSB is set to           */
                                    /*  communicate from the bootloader to the APP  */
                                    /*  that the errors in the field are fresh.     */
                                    /*  That is to say that the bootloader sets the */
                                    /*  MSB, the APP sees it and knows the error is */
                                    /*  fresh, so it can act on it and clear it     */
    
    IACM_LAST_LOCKOUT_REASON,       /* Store lockout reason to be cleared after     */
                                    /* lockout timer expires                        */
             
    // ~~~~~ -< add all items to the IACM above this line >- ~~~~~
    IACM_LAST_ITEM,
    // DO NOT EXCEED - 255 items above these comment lines
    IACM_CRC = 255,
    IACM_TOTAL,
}   IACM_ITEM_t;

uint32_t iacm_integrity_check(void);
uint32_t iacm_testbits(IACM_ITEM_t item, uint32_t tbits);
uint32_t iacm_get(IACM_ITEM_t item);
uint32_t iacm_base_addr_get(void);

void iacm_init(void);
uint32_t iacm_set(IACM_ITEM_t item, uint32_t val);
void iacm_set_from_fault(IACM_ITEM_t item, uint32_t val);
void iacm_set_from_isr(IACM_ITEM_t item, uint32_t val);
int iacm_setbits(IACM_ITEM_t item, uint32_t sbits);
int iacm_resetbits(IACM_ITEM_t item, uint32_t sbits);

//Mem corrupter commands
void iacm_corrupt_region(uint32_t region);
uint32_t iacm_get_crpt_rr_msk(void);


#ifdef	__cplusplus
}
#endif

#endif	/* LIB_IACM_H */


