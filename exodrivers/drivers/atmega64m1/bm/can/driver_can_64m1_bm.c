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
 * CAN_ATmega64m1_HAL.c
 *
 * Created: 9/25/2020 10:00:30 AM
 * Author : bwelker
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include <util/atomic.h>

#include "driver_can_64m1_bm.h"
#include "overflow_buffer.h"

/* This is CANMSG, which contains the CAN data byte pointed by the MOb page
 * register.  The CAN data buffer is seen as a FIFO.  The byte at this address
 * is equal to value at the specified index in CANPAGE:2:0. If
 * auto-incrementation is used (CANPAGE:3), after reading the value the index
 * (CANPAGE:2:0) will be auto-incremented.  Thus, the value of this register can
 * be read in a loop and the index will auto-increment.  Note, it will also
 * rollover, so if you start at index > 0, after reading index 7 it will wrap
 * back to zero.
 * 
 * This address will be passed as an argument in a callback function to any client
 * that signs up for a CAN Msg callback.  It is labeled as a 'fifo', so the
 * caller must understand that they must read the value of the pointer 
 * repeatedly, without incrementing the pointer.
 */
#define CAN_MSG_FIFO 0xFA
static volatile uint8_t *can_msg_fifo = (volatile uint8_t *)CAN_MSG_FIFO;

static volatile uint16_t err_code;
static volatile uint16_t err_code_hsi; /* HSI's copy, becuase err_code is a read and clear */
static volatile uint16_t can_msg_cnt;
static volatile uint8_t overflow_insert_cnt = 0;
static volatile uint8_t overflow_remove_cnt = 0;
extern volatile uint8_t ofb_head;
extern volatile uint8_t ofb_tail;

static health_table_entry_t health_stats[] = 
    DECLARE_CAN_HEALTH_ENTRY(&can_msg_cnt,          &err_code_hsi,
                             &overflow_insert_cnt,  &overflow_remove_cnt,
                             &ofb_head,             &ofb_tail);

typedef struct
{
	bool all;
	bool busOff;
	bool rx;
	bool tx;
	bool mobError;
	bool frameBufferError;
	bool generalErrors;
	bool timerOverrun;
}interruptEnables_t;

typedef enum
{
	CAN_MOB_CONFIG_DISABLE = 0,
	CAN_MOB_CONFIG_TX,
	CAN_MOB_CONFIG_RX,
	CAN_MOB_CONFIG_FRAME_BUFFER_RX,
	CAN_MOB_CONFIG_ENUM_SIZE
} canMobConfigurationEnum_t;


typedef struct
{
    bool used;
	bool canRev;
	uint8_t mobNum;
	uint8_t dataLength;
	uint8_t mobConfiguration;
	uint32_t deviceIdTag;
	uint32_t deviceIdMask;
	bool autoIncrement;
	bool mobInterruptEnable;
	bool remoteTransmissionRequest;
	bool replyValid;
	bool identifierTypeComparisonEnable;
    bool highPriorityMob;
    CAN_ISR_CALLBACK isrCallback;
} messageObject_t;

static interruptEnables_t interruptEnables = {
    .all = true,
    .rx = true,
    .tx = true,
};
    

/* private: */
static inline initReturnCodesEnum_t baudSet(const uint32_t baud);
static inline initReturnCodesEnum_t interruptEnable(interruptEnables_t* interruptEnables);
static inline initReturnCodesEnum_t mobControlAndDlcRegisterInit(messageObject_t* mob);
static inline initReturnCodesEnum_t mobDeviceIdTagSet(messageObject_t* mob);
static inline initReturnCodesEnum_t mobDeviceIdMaskSet(messageObject_t* mob);
static inline void pageSelect(messageObject_t* mob);
static inline void mobInterruptSet(messageObject_t* mob);
initReturnCodesEnum_t canMobInit(messageObject_t* mob);

/* Array (static memory) for MOBs - Maybe used or unused */
static messageObject_t mobs[MAX_MOBS] = {};
/* Stack of Free MOBs */
static messageObject_t *mob_stack[MAX_MOBS] = {};
/* Queue to handle messages in the order they arrive */
static messageObject_t *mob_queue[MAX_MOBS] = {};
/* Head of the queue - where to take message from */
static volatile uint8_t head = 0;
/* End of the queue - where to put messages */
static volatile uint8_t tail = 0;

static int mob_stack_top = -1;
#define STACK_EMPTY()       ((mob_stack_top == -1) ? 1:0)
#define STACK_FULL()        ((mob_stack_top == MAX_MOBS) ? 1:0)
#define FREE_MOB_COUNT()    ( mob_stack_top + 1 )

#define CAN_BUS_ERROR_BITMASK 0x1F
#define ERR_PUSH_MOB          0x80
#define ERR_MSG_BUFFERS_FULL  0x40


static messageObject_t *pop_mob()
{
    messageObject_t *mob = NULL;
    if (!STACK_EMPTY()) {
        mob = mob_stack[mob_stack_top];
        mob_stack_top = mob_stack_top - 1;
    }
    return mob;
}

static int push_mob(messageObject_t *mob) {
    int err = 0;
    if (!STACK_FULL()) {
        mob_stack_top++;
        mob_stack[mob_stack_top] = mob;
    } else {
        err = ERR_PUSH_MOB;
    }
    return err;
}

/**
 * Initialize CAN peripheral
 * 
 * @note ttcEnable and ttcSync are usually set to true 
 * 
 * 
 * @param ttcEnable Enable time trigger communication (TTC) 16 bit timer
 * @param ttcSync Synchronize TTC timer at end of CAN frame
 * @param baud Bit rate for CAN comms
 * @param interruptEnables Struct for setting which events trigger interrupts
 * @return status return code
 */
uint8_t canControllerInit(const bool ttcEnable, const bool ttcSync, const uint32_t baud) {
    // Sw reset controller
    CANGCON = (1 << SWRES);

    // Set time trigger control and frame synchronization
    if (ttcEnable) {
        CANGCON |= (1 << TTC);
    }

    if (ttcSync) {
        CANGCON |= (1 << SYNTTC);
    }

    initReturnCodesEnum_t status = SUCCESS;

    // Set baud
    if ((status = baudSet(baud)) != SUCCESS) {
        return status;
    }

    // Set interrupts
    if ((status = interruptEnable(&interruptEnables)) != SUCCESS) {
        return status;
    }

    for (int i = 0; i < MAX_MOBS; i++) {
        /* Make sure the MOBs are numbered in the order they are in
           the array, and that it matches the HW's concept of MOB numbers */
        mobs[i].mobNum = i;
    }

    /* Init & push all the MOBs onto the free stack */
    for (int i = 0; i < MAX_MOBS; i++) {
        /* Push them onto the stack so that lower MOBs are on top */
        messageObject_t *mob = &mobs[(MAX_MOBS - 1) - i];
        mob->used = false;
        mob->canRev = CAN_REV_A;
        mob->dataLength = 8;
        mob->mobConfiguration = 0;
        mob->deviceIdTag = 0;
        mob->deviceIdMask = 0;
        mob->autoIncrement = true;
        mob->mobInterruptEnable = true;
        mob->remoteTransmissionRequest = false;
        mob->replyValid = false;
        mob->identifierTypeComparisonEnable = false;
        mob->isrCallback = NULL;
        canMobInit(mob);
        push_mob(mob);
    }
    
    // Enable controller
    CANGCON |= (1 << ENASTB);
    
    // Wait for controller to enable
    while (!(CANGSTA & (1 << ENFG))) {
    }

    return status;
}


///////////////////////////////////////////////////////////////////////////////
// Set CAN baud.  Units are Kbits/sec.
//
// Return status
static inline initReturnCodesEnum_t baudSet(const uint32_t baud) {
    switch (baud) {
        case CAN_BAUD_RATE_100:
            CANBT1 = 0x12;
            CANBT2 = 0x04;
            CANBT3 = 0x13;
            break;

        case CAN_BAUD_RATE_125:
            CANBT1 = 0x0E;
            CANBT2 = 0x04;
            CANBT3 = 0x13;
            break;

        case CAN_BAUD_RATE_200:
            CANBT1 = 0x08;
            CANBT2 = 0x04;
            CANBT3 = 0x13;
            break;

        case CAN_BAUD_RATE_250:
            CANBT1 = 0x06;
            CANBT2 = 0x04;
            CANBT3 = 0x13;
            break;

        case CAN_BAUD_RATE_500:
            CANBT1 = 0x02;
            CANBT2 = 0x04;
            CANBT3 = 0x13;
            break;

        case CAN_BAUD_RATE_1000:
            CANBT1 = 0x00;
            CANBT2 = 0x04;
            CANBT3 = 0x12;
            break;

        default:
            return CAN_INIT_INVALID_BAUD_RATE;
            break;
    }

    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Set CAN bus interrupts
//
// Return status
static inline initReturnCodesEnum_t interruptEnable(interruptEnables_t* interruptEnables) {
    if (interruptEnables == NULL) {
        return CAN_INIT_INTERRUPT_ENABLES_STRUCT_NULL;
    }


    CANGIE = RESET;

    bool* structMemberPtr = &interruptEnables->all;

    // Loop through struct and set interrupt if field is set to true
    for (uint8_t interruptEnablesBit = 0; interruptEnablesBit < 8; interruptEnablesBit++) {
        if (true == (*(structMemberPtr + interruptEnablesBit))) {
            CANGIE |= (1 << (7 - interruptEnablesBit));
        }
    }

    return SUCCESS;
}


/**
 * Load data from local buffer to CAN frame data buffer
 * 
 * @note local buffer must be 8 bytes.  Frame buffer will wrap if local buffer
 * is longer
 * 
 * 
 * @param mob Message object data structure to load memory into
 * @param data local buffer to load data from
 * @return void
 */
static void canMobDataSet(messageObject_t* mob, uint8_t* data) {
    pageSelect(mob);
    if (!mob->autoIncrement) {
        /* disables auto increment */
        CANPAGE |= (1 << AINC);
    }

    CANPAGE &= ~(0x07);
    for (uint8_t i = 0; i < mob->dataLength; i++) {
        CANMSG = data[i];
    }
}


/**
 * Initialize general use message object 
 * 
 * @note Typically used for TX mobs
 * 
 * @param mob Pointer to message object data structure 
 * @return status return code
 */
initReturnCodesEnum_t canMobInit(messageObject_t* mob) {
    if (mob == NULL) {
        return MOB_INIT_NULL_ARG;
    }

    if (mob->mobNum > (MAX_MOBS - 1)) {
        return MOB_INIT_INVALID_MOB_NUM;
    }

    initReturnCodesEnum_t status = SUCCESS;

    pageSelect(mob);

    
    if ((status = mobControlAndDlcRegisterInit(mob)) != SUCCESS) {
        return status;
    }

    if ((status = mobDeviceIdTagSet(mob)) != SUCCESS) {
        return status;
    }

    if ((status = mobDeviceIdMaskSet(mob)) != SUCCESS) {
        return status;
    }

    mobInterruptSet(mob);
    uint8_t reset_data[8] = {0};
    canMobDataSet(mob, reset_data);
    CANSTMOB = RESET;
    
    return status;
}


/**
 * Initialize RX mob - Make sure interrupts are off!
 * 
 * 
 * @param count Number of RX mobs to initialize
 * @param id Base CAN ID that the mob will listen for
 * @param mask What mobs to allow other than base id
 * @param cb Callback function for high priority message handling
 * @return err return code
 */
initReturnCodesEnum_t canMobInitRx(uint8_t count, uint32_t id, 
                                   uint32_t mask, CAN_ISR_CALLBACK cb)
{
    int err = SUCCESS;
    messageObject_t *mob = NULL;

    if (count > FREE_MOB_COUNT()) {
        err = MOB_INIT_NO_FREE_MOBS;
    }
    /* For the number of MOBs requested */
    for (int n = 0; (n < count) && (err == SUCCESS); n++) {
        mob = pop_mob();
        if (mob != NULL) {
            mob->mobConfiguration = CAN_MOB_CONFIG_RX;
            mob->deviceIdTag = id;
            mob->deviceIdMask = mask;
            if(cb != NULL){
                mob->isrCallback = cb;
            }
            err = canMobInit(mob);

            if (err == SUCCESS) {
                mob->used = true;
            }
        }
    }
    return err;
}


/**
 * Get number of mobs available on mob stack
 *
 * @return number of available mobs
 */
uint8_t canFreeMobs(void) {
    return FREE_MOB_COUNT();
}


///////////////////////////////////////////////////////////////////////////////
// Select page
//
// Return void
static inline void pageSelect(messageObject_t* mob) {
    CANPAGE = (mob->mobNum << 4);
}


///////////////////////////////////////////////////////////////////////////////
// Initialize CAN control and data length code register
//
// Return status
static inline initReturnCodesEnum_t mobControlAndDlcRegisterInit(messageObject_t* mob) {
    CANCDMOB = RESET;
    if (mob->mobConfiguration > CAN_MOB_CONFIG_FRAME_BUFFER_RX) {
        return MOB_INIT_INVALID_MOB_CONFIGURATION;
    }

    if (mob->dataLength > MAX_DATA_LENGTH) {
        return MOB_INIT_INVALID_DATA_LENGTH;
    }

    CANCDMOB |= (mob->mobConfiguration << 6);

    if (mob->canRev == CAN_REV_B) {
        CANCDMOB |= (1 << IDE);
    }

    CANCDMOB |= (mob->dataLength);

    if (mob->replyValid) {
        CANCDMOB |= (1 << RPLV);
    }

    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Initialize device id
//
// Return status

static inline initReturnCodesEnum_t mobDeviceIdTagSet(messageObject_t* mob) {
    CANIDT1 = RESET;
    CANIDT2 = RESET;
    CANIDT3 = RESET;
    CANIDT4 = RESET;
    if (mob->canRev == CAN_REV_A) {
        if (mob->deviceIdTag > 0x7ff) {
            return MOB_INIT_INVALID_DEVICE_ID_TAG;
        }

        CANIDT1 = (mob->deviceIdTag >> 3) & (0xFF);
        CANIDT2 = (mob->deviceIdTag << 5) & (0xE0);

        if (mob->remoteTransmissionRequest) {
            CANIDT4 |= (1 << RTRTAG);
        }
    } else /* canRev = CAN_REV_B */ {
        if (mob->deviceIdTag > 0xFFFFFFF8) {
            return MOB_INIT_INVALID_DEVICE_ID_TAG;
        }

        CANIDT1 = (mob->deviceIdTag >> 24) & 0xFF;
        CANIDT2 = (mob->deviceIdTag >> 16) & 0xFF;
        CANIDT3 = (mob->deviceIdTag >> 8) & 0xFF;
        CANIDT4 = (mob->deviceIdTag) & 0xF8;

        if (mob->remoteTransmissionRequest) {
            CANIDT4 |= (1 << RTRTAG);
        }
    }

    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Initialize device id mask
//
// Return status

static inline initReturnCodesEnum_t mobDeviceIdMaskSet(messageObject_t* mob) {
    CANIDM1 = RESET;
    CANIDM2 = RESET;
    CANIDM3 = RESET;
    CANIDM4 = RESET;
    if (mob->canRev == CAN_REV_A) {
        if (mob->deviceIdMask > 0x7ff) {
            return MOB_INIT_INVALID_DEVICE_ID_MASK;
        }

        CANIDM1 = (mob->deviceIdMask >> 3) & (0xFF);
        CANIDM2 = (mob->deviceIdMask << 5) & (0xE0);

        if (mob->remoteTransmissionRequest) {
            CANIDM4 |= (1 << RTRTAG);
        }

        if (mob->identifierTypeComparisonEnable) {
            CANIDM4 |= IDEMSK;
        }
    } else /* canRev = CAN_REV_B */ {
        if (mob->deviceIdMask > 0xFFFFFFF8) {
            return MOB_INIT_INVALID_DEVICE_ID_MASK;
        }

        CANIDM1 = (mob->deviceIdMask << 24) & 0xFF;
        CANIDM2 = (mob->deviceIdMask << 16) & 0xFF;
        CANIDM3 = (mob->deviceIdMask << 8) & 0xFF;
        CANIDT4 = (mob->deviceIdMask) & 0xF8;

        if (mob->remoteTransmissionRequest) {
            CANIDT4 |= (1 << RTRTAG);
        }

        if (mob->identifierTypeComparisonEnable) {
            CANIDT4 |= IDEMSK;
        }
    }

    return SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// Set Mob interrupts
//
// Return void

static inline void mobInterruptSet(messageObject_t* mob) {
    if (mob->mobInterruptEnable) {
        CANIE2 |= (1 << mob->mobNum);
    }
}


/**
 * Take mob out of disabled state
 * 
 * @note tx mobs are automatically disabled after use
 * 
 * 
 * @param mob Message object data structure
 * @param configuration Configuration to put mob in
 * @return void
 */
static void canMobResume(messageObject_t* mob, canMobConfigurationEnum_t configuration) {
    pageSelect(mob);
    CANCDMOB &= ~((1 << CONMOB0) | (1 << CONMOB1)); /*clear old configuration*/
    CANCDMOB |= (configuration << 6);
    /* Only enable interrupts for RX (for now) */
    if (mob->mobInterruptEnable || configuration == CAN_MOB_CONFIG_RX) {
        CANIE2 |= (1 << mob->mobNum);
    }
}


/**
 * Get timestamp for CAN TTC register
 * 
 * @return timestamp
 */
uint16_t timestampGet(void) {
    return (CANSTMH << 8 | CANSTML);
}


/**
 * Receive incoming CAN message
 * 
 * @note typically polled on in main hardware loop
 * 
 * 
 * @param rxBuffer, local buffer to receive can message data into
 * @return true if data successfully received and ready to be used
 */
bool command_rx(uint8_t* rxBuffer) {
    bool data_ready = false;
    if (head != tail) {
        messageObject_t *mob = mob_queue[head];
        head = (head + 1) % MAX_MOBS;
        /* Access the page buffer for this mob (0 reference, 1 is mob zero page) */
        CANPAGE = (mob->mobNum << 4);
        for (uint8_t i = 0; i < (CANCDMOB & (0x0F)); i++) {
            rxBuffer[i] = CANMSG;
        }
        canMobResume(mob, CAN_MOB_CONFIG_RX);
        data_ready = true;
    }
    return data_ready;
}

/**
 * Send the mod
 * @param mob pointer to message object
 * @param id CAN id
 * @param dlc length of message
 * @param data data to send
 */
static void can_send_mob(messageObject_t *mob, uint16_t id, int dlc, uint8_t *data)
{
    mob->used = true;
    mob->canRev = CAN_REV_A;
    mob->dataLength = dlc;
    mob->mobConfiguration = CAN_MOB_CONFIG_DISABLE;
    mob->deviceIdTag = id;
    mob->deviceIdMask = 0x7FF;
    mob->autoIncrement = true;
    mob->mobInterruptEnable = true;
    mob->remoteTransmissionRequest = false;
    mob->replyValid = false;
    mob->identifierTypeComparisonEnable = false;
    mob->isrCallback = 0;
    canMobInit(mob);
    canMobDataSet(mob, data);
    canMobResume(mob, CAN_MOB_CONFIG_TX);
}

/**
 * Send a message on the CAN bus. Save message in overflow buffer if
 * no MOBs are available.
 * 
 * @param id CAN id to set in frame
 * @param txbuffer Data to send
 * @return 0 if sending (or queue'd up for later) non-zero if them message drops
 */
int can_send_msg(uint32_t id, int dlc, uint8_t *data)
{
    messageObject_t *mob = NULL;
    int err = 0;
    
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 
    {
        mob = pop_mob();
        if(mob == NULL) {
            /* Disable TX interrupts to protect stack operations */
            err = ofb_message_save(id, dlc, data);
            if(err) {
                err_code |= ERR_MSG_BUFFERS_FULL;
            } else {
                overflow_insert_cnt++;
            }
        }
    }
    if(mob) {
        can_send_mob(mob, id, dlc, data);
    }
    return err;
}

/**
 * Legacy send message that assumes we only can send 8 byte CAN messages
 * 
 * @note txbuffer must be 8 bytes for successful transmission
 * 
 * @param id message id
 * @param txbuffer data buffer
 * @return true if sending (or queue'd up for later) false if them message drops
 */
bool canSendMsg(uint32_t id, uint8_t *txbuffer)
{
    return can_send_msg(id, 8, txbuffer) == 0;
}

/**
 * Get error codes from ISR
 * 
 * @note Will return -1 on stack error, 
 * 0 on success or isr_err_code & CAN_BUS_ERROR_BITMASK
 * 
 * 
 * @return error status from ISR
 */
uint16_t can_err_code_get(void)
{
    int ret = err_code;
    err_code = 0;
    return ret;
}

/**
 * Return HSI stats.  CAN errors are cleared on read
 * @return 
 */
health_table_entry_t *can_health_get(void)
{
    err_code_hsi = err_code;
    err_code = 0;
    return health_stats;
}

static uint16_t get_rx_can_id(void) {
    uint16_t can_id_lw = 0;
    uint16_t can_id_hw = 0;
    can_id_lw = CANIDT2 >> 5;
    can_id_hw = CANIDT1 << 3;
    
    uint16_t can_id;
    can_id = can_id_lw | can_id_hw;
    return can_id;
}

///////////////////////////////////////////////////////////////////////////////
// ISR for successful RX hit
//
// Return void
volatile uint8_t hipri_rx_buffer[8] = {}; // FIXME - For debug, don't optimize

ISR(CAN_INT_vect) {
    
    // If there's a bus error, stop.
    if(CANSTMOB & CAN_BUS_ERROR_BITMASK){
        err_code |= CANSTMOB & CAN_BUS_ERROR_BITMASK;
        CANSTMOB &= ~CAN_BUS_ERROR_BITMASK;
    } else {
        /* This is an ISR, save current page and restore at the end so
           we are not pulling the rug from under the other routines */
        uint8_t saved_page = CANPAGE;

        /* The highest priority MOB number in CANSIT is stored in CANHPMOB 4:7
         *  and CANHPMOB mirrors CANPAGE to allow the MOB to be selected directly
         *  0xf means no more outstanding messages */
        while ((((CANPAGE = CANHPMOB) >> 4) != 0xF)) {

            /* Disable the MOB */
            CANCDMOB &= ~((1 << CONMOB1) | (1 << CONMOB0));

            /* disable interrupts for this MOB until processed later */
            uint8_t hpmob = (CANHPMOB >> 4) & 0xF;
            CANIE2 &= (~(1 << hpmob) & 0xFF);

            messageObject_t* mob = &mobs[hpmob];

            if (CANSTMOB & (1 << TXOK)) {
                can_msg_cnt++;
                /* TX completed for this page - clear interrupt, disable page */
                CANSTMOB &= ~(1 << TXOK);
                if(!ofb_empty()) {
                    uint16_t id;
                    uint8_t dlc;
                    uint8_t data[] = {0,0,0,0,0,0,0,0};
                    ofb_message_get(&id, &dlc, data);
                    can_send_mob(mob, id, dlc, data);
                } else {
                    mob->used = false;
                    err_code |= push_mob(mob);
                }
            } else if (CANSTMOB & (1 << RXOK)) {
                can_msg_cnt++;
                /* Acknowledge Rx interrupt */
                CANSTMOB &= ~(1 << RXOK);
                /* Receive completed for this MOB */
                if (mob->isrCallback) {
                    /* high pri MOB - do it now! */
                    /* Make sure the Index is set to zero & Auto-increment is enabled */
                    CANPAGE = (mob->mobNum << 4);
                    /* lower 4 bit of CANCDBMOB is DLC */
                    uint8_t dlc = (CANCDMOB & (0x0F));
                    mob->deviceIdTag = get_rx_can_id();
                    mob->isrCallback(mob->deviceIdTag, dlc, can_msg_fifo);
                    canMobResume(mob, CAN_MOB_CONFIG_RX);
                } else {
                    /* Else - Queue the MOB for the commandRx function */
                    mobs[hpmob].deviceIdTag = get_rx_can_id();
                    mob_queue[tail] = &mobs[hpmob];
                    tail = (tail + 1) % MAX_MOBS;
                }

            } else {
                // FIXME -- Some error?
            }
        }
        CANPAGE = saved_page;
    }
}
