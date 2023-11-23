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

#define F_CPU 8000000
#include <avr/boot.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#include "mcu_include.h"
#include "CAN_ATmega64m1_HAL.h"
#include "CAN_ATmega64m1_Bootloader.h"
#include "checksum.h"

#define FLASH_PAGE_SIZE_BYTES    0x0100
#define FLASH_PAGE_ADDRESS_ZERO  0x0000
#define END_APPLICATION_FLASH    0xE000
#define MAX_CAN_DATA_BYTES       0x08
#define WORD_SIZE_BYTES          0x02
#define COMMAND_OK               0x00

/* Strapping resistor values */
typedef enum{
    ERROR          = 0,
    KEEPER         = 1,
    VALVES         = 2,
    MAGNET_OUTER   = 3,
    ANODE          = 4,
    BUCK           = 5,
    MAGNET_INNER   = 7,
} atmega_boot_ids_t;

typedef struct bootloader_stat {
    uint8_t magic;
    uint32_t app_version;
    uint16_t size;
    uint16_t image_crc;
    uint16_t bootloader_stat_crc;
} bootloader_stat_t;

typedef union bootloader_info{
    bootloader_stat_t bl_stat;
    uint8_t bl_stat_byte_buffer[sizeof(bootloader_stat_t)];
} union_bootloader_info_t;

typedef struct  
{
    uint8_t rxBuffer[MAX_CAN_DATA_BYTES];
    bootloaderState_t bootloaderState;
    uint16_t programStartAddress;
    uint16_t programEndAddress;
    uint16_t flashReadStartAddress;
    uint16_t flashReadEndAddress;
    uint8_t  nodeId;
    uint16_t crc;
    uint32_t app_version;
} bootloader_t;


#define SIZEOF(__X__) (sizeof(__X__)/sizeof(__X__[0]))

#define SIZEOF_MINUS_CRC(__X__, __Y__) (sizeof(__X__) - sizeof(__Y__))

/* Read port values to determine board IDs*/
#define CAN_CRIS_ID() \
        ((PINC & (1 << PINC1)) << 1)     | \
        ((PIND & (1 << PIND1)))          | \
        ((PINB & (1 << PINB4)) >> 4)       \
        
/* Byte address for end of application flash when using 4096 (16 bit) words
 * to store bootloader
 */
#define END_OF_FLASH_BYTE_ADDR 0xE000
#define LAST_PAGE_APP_FLASH (END_OF_FLASH_BYTE_ADDR - FLASH_PAGE_SIZE_BYTES)
#define BL_STAT_FLASH_START (END_OF_FLASH_BYTE_ADDR - sizeof(bootloader_stat_t))

#define BL_STAT_READ  0
#define BL_STAT_WRITE 1

#define BL_MAGIC 0xAA

void flashPageWrite(uint8_t* programData, uint16_t page);
uint8_t commandProcess(bootloader_t* bootloader);

/*
 * Helper function to read/write bootloader info 
 */

/* TODO fill in doxy info */
/**
 * 
 * @param bootloader_stat
 * @param opt
 * @return 
 */
static int bootloader_stat(union_bootloader_info_t* bootloader_stat, int opt)
{
    int err = 0;
    /* Preconditions */
    if(bootloader_stat == NULL || (opt != BL_STAT_READ && opt != BL_STAT_WRITE)){
        err = __LINE__;
    } 
    if(!err && (opt == BL_STAT_READ)){
        /* Shouldn't really need variables for these, but the debugger disagrees. 
         */
        int bl_flash_Start = BL_STAT_FLASH_START;
        for(int i = BL_STAT_FLASH_START; i < END_OF_FLASH_BYTE_ADDR; i++){
            bootloader_stat->bl_stat_byte_buffer[i - bl_flash_Start] = pgm_read_byte(i);
        }
    } else if(!err && (opt == BL_STAT_WRITE)){
        uint8_t flash_page_buffer[FLASH_PAGE_SIZE_BYTES] = {0}; 
        
        /* Read and store last page to make sure we don't lose anything */
        for(int i = LAST_PAGE_APP_FLASH; i < END_OF_FLASH_BYTE_ADDR; i++){
            flash_page_buffer[i - LAST_PAGE_APP_FLASH] = pgm_read_byte(i);
        }
         /* Again, Shouldn't really need variables for these */
        int bl_flash_Start = BL_STAT_FLASH_START;
        int eof = END_OF_FLASH_BYTE_ADDR;
        int bl_index = FLASH_PAGE_SIZE_BYTES - (eof - bl_flash_Start);

        /* Copy bootloader data into buffer*/
        memcpy(&flash_page_buffer[bl_index], 
              bootloader_stat, sizeof(union_bootloader_info_t));

        flashPageWrite(flash_page_buffer, LAST_PAGE_APP_FLASH);
    }
    
    return err;
}

/**
 * Announce presence by using the CRIS ID + 0x7, and announce the version of
 * firmware currently loaded, and give status on the app.
 * 
 * The command ID of 0x7 is not used in the example, neither is 
 * announcing the presence of new device.  We need to do that in 
 * (at least) the case where an Atmega resets without the controllers
 * knowledge.
 * 
 * @param can_id MCU base CAN ID
 * @param hdr_status the status of the header
 * @param bl_info pointer to bootloader header info
 */
static inline void bootloader_initial_message(int can_id, uint8_t hdr_status, union_bootloader_info_t *bl_info)
{    
    bootloader_initial_info_t info;
    
    canMobInitRx(1, can_id, CRIS_COMMAND_MASK, NULL);
    
    info.msg.bootloader_version = BOOTLOADER_VERSION;
    info.msg.app_version = bl_info->bl_stat.app_version;
    info.msg.app_crc = bl_info->bl_stat.image_crc;
    info.msg.header_stat = hdr_status;
    
    can_id |= CRIS_COMMAND_ID_NEW_BOOT;
    canSendMsg(can_id, info.data);
    can_id &= CRIS_COMMAND_MASK;
}

/**
 * This will read a header that is maintained at the end of the flash.  That
 * header has its own CRC, and contains the CRC for the data area of the
 * firmware app and the app's version.  This will read the header, validate it,
 * then use the info in the header to validate the app area.
 * 
 * @param bl_info pointer to bootloader info structure
 * @return header status see bootloader_header_err_t
 */
static inline bootloader_header_err_t bootloader_read_header(union_bootloader_info_t *bl_info)
{
    int err;
    
    // Mark as unknown err, so that this HAS to validate the header completely
    // before reporting BL_HDR_OK.  If it is mark BL_HDR_OK by default, and
    // an error case is missed below, it will erroneously report it is ok.
    uint8_t bl_hdr_status = BL_HDR_UNKNOWN_ERR;
    
    /* Read bootloader header */
    err = bootloader_stat(bl_info, BL_STAT_READ);
    if(err) {
        bl_hdr_status = BL_HDR_READ_ERR;
    } else if(bl_info->bl_stat.magic == 0xFF){
        // Header is not initialized
        bl_hdr_status = BL_HDR_IMAGE_UNINITIALIZED;
        
    } else if(bl_info->bl_stat.magic != BL_MAGIC) {
        // Something was written here, but it is invalid
        bl_hdr_status = BL_HDR_INVALID_MAGIC;
        
    } else {
        // Header magic is good, check the header CRC
        
        uint16_t crc = 0;

        for(int i = 0; i < SIZEOF_MINUS_CRC(union_bootloader_info_t, 
                bl_info->bl_stat.bootloader_stat_crc); i++){
            crc = update_crc_16(crc, bl_info->bl_stat_byte_buffer[i]);
        }
        if(crc != bl_info->bl_stat.bootloader_stat_crc){
            bl_hdr_status = BL_HDR_CRC_MISMATCH;
        
        } else{
            // Header is valid - check the data area
            crc = 0;
            for(uint16_t i = 0; i < bl_info->bl_stat.size; i++){
                uint8_t image_byte = pgm_read_byte(i);
                crc = update_crc_16(crc, image_byte);
            }

            if(crc == bl_info->bl_stat.image_crc){
                bl_hdr_status = BL_HDR_OK;
            } else {
                bl_hdr_status = BL_HDR_IMAGE_CRC_MISMATCH;
            }
        }
    }
    return bl_hdr_status;
}

//////////////////////////////////////////////////////////////////////////////////
/// Main
///
int main (void)
{    
    // disable interrupts for clock change
    cli();
# if 0
    // enable clock prescaler change
    CLKPR = (1 << CLKPCE);
    // divide clock by two, resulting in a 8MHz clock
    CLKPR = (1 << CLKPS0);
#endif
    
    // Enable change of Interrupt Vectors 
    MCUCR = (1<<IVCE);
    // Move interrupts to Boot Flash section 
    MCUCR = (1<<IVSEL);

    /* Init CAN peripheral and enable global interrupts*/
    canControllerInit(false, false, CAN_BAUD_RATE_1000);
    sei();
    
    /* Get resistor strapping value */
    volatile uint8_t  resistor_id = 0x5;//CAN_CRIS_ID();
    uint8_t  node_id = 0x00;
    uint16_t can_id  = 0x00;
    
    int err = 0;
    /* Determine who we are */
    switch(resistor_id){
        case KEEPER:
            can_id  = BL_CRIS_ID_KEEPER;
            node_id = BL_NODE_ID_KEEPER;
            break;
            
        case VALVES:
            can_id  = BL_CRIS_ID_VALVES;
            node_id = BL_NODE_ID_VALVES;
            break;
            
        case MAGNET_OUTER:
            can_id  = BL_CRIS_ID_MAGNET_O;
            node_id = BL_NODE_ID_MAGNET_O;
            break;
            
        case ANODE:
            can_id  = BL_CRIS_ID_ANODE;
            node_id = BL_NODE_ID_ANODE;
            break;
            
        case MAGNET_INNER:
            can_id  = BL_CRIS_ID_MAGNET_I;
            node_id = BL_NODE_ID_MAGNET_I;
            break;
            
        case BUCK:
            can_id  = BL_CRIS_ID_BUCK;
            node_id = BL_NODE_ID_BUCK;
            break;            
            
        default:
            /* Identity crisis */
            err = __LINE__;
            break;
    }
    
    // Variables that need to retain values through iterations of main 
    //  firmware loop
    uint16_t programDataIndex = 0;
    uint16_t readFlashIndex = 0;
    uint8_t programData[FLASH_PAGE_SIZE_BYTES] = {0};
    uint16_t page = FLASH_PAGE_ADDRESS_ZERO;
    bool send_success = true;
    bool rcv_success = true;
    uint8_t image_byte;
    uint16_t image_crc = 0;
    
    bootloader_t bootloader = {.rxBuffer = {0x00},
                               .bootloaderState = IDLE,
                               .programStartAddress = 0,
                               .programEndAddress = 0,
                               .flashReadStartAddress = 0,
                               .flashReadEndAddress = 0,
                               .nodeId = node_id};

    


    uint16_t bytes_received = 0;
    uint8_t bl_hdr_status = BL_HDR_OK;
    union_bootloader_info_t bl_info = {0};
    
    bl_hdr_status = bootloader_read_header(&bl_info);
    bootloader_initial_message(can_id, bl_hdr_status, &bl_info);
    
    // IF we had an error reading the header, it has been reported, we
    //  will now try to boot load and accept a new image anyway.  Else, the
    //  MCU may never try to rewrite the flash
    err = 0;
    
    while((bootloader.bootloaderState != APPLICATION_START) && !err)
    {
        // Successful CAN RX hit
        
        // Variables that can fall off the stack through iterations of main
        //  firmware loop
        uint8_t replyBuffer[MAX_CAN_DATA_BYTES] = {0};
        volatile uint16_t status = COMMAND_OK;
        
        rcv_success = command_rx(bootloader.rxBuffer);
        if(rcv_success == true) //  Successful CAN RX hit
        {   
            status = commandProcess(&bootloader); // Adjust state variable/bootloader status
            
            if(bootloader.bootloaderState == PROGRAMMING)
            {
                // Write CAN message into program memory buffer
                memcpy(&programData[programDataIndex], bootloader.rxBuffer, MAX_CAN_DATA_BYTES);
                bytes_received += SIZEOF(bootloader.rxBuffer);
                int size = SIZEOF(bootloader.rxBuffer);
                
                if(bytes_received > bootloader.programEndAddress){
                    size = (bootloader.programEndAddress - bytes_received) + 
                            SIZEOF(bootloader.rxBuffer);
                }
                
                for(int i = 0; i < size; i++){
                    image_crc = update_crc_16(image_crc, bootloader.rxBuffer[i]);
                }
                
                // Set status variable
                status = BL_RPLY_PROG_DATA_OK_MORE;
                
                // Adjust data index for next 8 bytes of program data
                programDataIndex = (programDataIndex + MAX_CAN_DATA_BYTES);
                
                // if data to write is less than a full page => program is done being written
                // if data to write is greater than page size -> more data to write 
                if(programDataIndex >= (bootloader.programEndAddress - page) || programDataIndex >= FLASH_PAGE_SIZE_BYTES)
                {
                    // Write page
                    flashPageWrite(programData, page);

                    // Increment page address
                    page += FLASH_PAGE_SIZE_BYTES;
                    
                    // Reset program data buffer
                    memset(programData, 0xFF, FLASH_PAGE_SIZE_BYTES);
                    // Reset index counter
                    programDataIndex = 0;
                }
                
                
                // If page address is greater than address range => program is done being written
                if(page >= (bootloader.programEndAddress - bootloader.programStartAddress))
                {
                    if(image_crc == bootloader.crc){
                        memset(programData, 0xFF, FLASH_PAGE_SIZE_BYTES);

                        // Update state variable
                        bootloader.bootloaderState = PROGRAMMING_DONE;

                        // Return status for reply
                        status = BL_RPLY_PROG_DATA_OK_DONE;

                        /* Update bootloader header info */
                        bl_info.bl_stat.image_crc = image_crc;
                        
                        /* If this is our first update with a new board */
                        bl_info.bl_stat.magic = BL_MAGIC;
                        
                        bl_info.bl_stat.size = 
                                bootloader.programEndAddress - 
                                bootloader.programStartAddress;
                        
                        bl_info.bl_stat.app_version = bootloader.app_version;
                        
                        /* Zero out crc for new crc calculation */
                        bl_info.bl_stat.bootloader_stat_crc = 0;
                        for(int i = 0; i < SIZEOF_MINUS_CRC(union_bootloader_info_t, 
                                        bl_info.bl_stat.bootloader_stat_crc); i++){
                            bl_info.bl_stat.bootloader_stat_crc = 
                                    update_crc_16(bl_info.bl_stat.bootloader_stat_crc, 
                                    bl_info.bl_stat_byte_buffer[i]);
                        }
                        
                        // zero out crc to re-use for flash read crc check
                        image_crc = 0;
                        err = bootloader_stat(&bl_info, BL_STAT_WRITE);
                        
                    } else {
                        status = BL_RPLY_CMD_OK_XFER_CRC_FAILED;
                        err = status = __LINE__;
                    }
                }
                replyBuffer[BL_RPLY_STATUS_INDEX] = status; 
                can_id |= CRIS_COMMAND_ID_PROG_DATA;
            }  


            if(bootloader.bootloaderState == READING_MEMORY)
            {
                /* On request from SAM, readback app data written to flash and verify crc*/
                if(!err){
                    for(uint16_t i = bootloader.programStartAddress; i < bootloader.programEndAddress; i++){
                        image_byte = pgm_read_byte(i);
                        image_crc = update_crc_16(image_crc, image_byte);
                    }

                    if(image_crc == bootloader.crc){
                        /* if we get to here it's pretty likely that we have a good image
                         * Send to SAM for final verification and app-start "go-ahead"
                         */
                        *((uint16_t*)&replyBuffer[BL_RPLY_INDEX_READBACK_CRC]) = 
                                bl_info.bl_stat.image_crc;
                        
                        *((uint32_t*)&replyBuffer[BL_RPLY_INDEX_APP_NEW_VERSION]) = 
                                bl_info.bl_stat.app_version; 

                        status = BL_RPLY_READBACK_DONE;
                        
                    } else {
                        status = BL_RPLY_CMD_OK_READBACK_CRC_FAILED;
                        err = __LINE__;
                    }
                }

                replyBuffer[BL_RPLY_STATUS_INDEX] = status;

                // Update state variable
                bootloader.bootloaderState = PROGRAMMING_DONE;

                readFlashIndex = 0;
                can_id |= CRIS_COMMAND_ID_DISPLAY_DATA;
            }


            if(bootloader.bootloaderState == ERASING)
            {
                /* Don't erase last page of flash containing bootloader info */
                for(uint16_t pageToErase = 0; pageToErase < BL_STAT_FLASH_START; pageToErase += FLASH_PAGE_SIZE_BYTES)
                {
                    boot_page_erase(pageToErase);
                }
                bootloader.bootloaderState = ERASED;
                // Command OK - Start programming
                can_id |= CRIS_COMMAND_ID_PROG_START;

            }


            if(bootloader.bootloaderState == BLANK_CHECKING)
            {
                if(readFlashIndex <= bootloader.flashReadEndAddress)
                {
                    for(uint16_t blankCheckBytes = 0; blankCheckBytes < bootloader.flashReadEndAddress; blankCheckBytes++)
                    {
                        if(pgm_read_byte(bootloader.flashReadStartAddress + readFlashIndex++) != 0xFF)
                        {
                            status = bootloader.flashReadStartAddress + readFlashIndex - 1; // subtract 1 to account for post incrememnt
                            // Set status field in reply
                            replyBuffer[BL_RPLY_STATUS_INDEX] = (status  >> 8) & 0xFF;
                            replyBuffer[BL_RPLY_STATUS_INDEX + 1] = (status) & 0xFF;
                            break;
                        }
                    }
                }
                readFlashIndex = 0;
                bootloader.bootloaderState = BLANK_CHECKED;
                can_id |= CRIS_COMMAND_ID_DISPLAY_DATA;
            }

            // FIX ME - this should all be push up above to individual states 
            if(bootloader.bootloaderState == PROGRAMMING_MODE_INITIALIZE)
            {
                memset(&bl_info, 0, sizeof(union_bootloader_info_t));
                volatile int err = bootloader_stat(&bl_info, BL_STAT_READ);
                if(!err){
                    replyBuffer[BL_RPLY_INDEX_BL_VERSION]    = BOOTLOADER_VERSION;
                    *((uint32_t*)&replyBuffer[BL_RPLY_INDEX_APP_NEW_VERSION]) = bl_info.bl_stat.app_version;
                    replyBuffer[BL_RPLY_INDEX_INIT_STATUS]   = status;
                    replyBuffer[BL_RPLY_INDEX_BL_HDR_STATUS] = bl_hdr_status;
                }
            }
            else if(bootloader.bootloaderState == PROGRAMMING_MODE_START)
            {
                replyBuffer[BL_RPLY_STATUS_INDEX] = status;
                can_id |= CRIS_COMMAND_ID_PROG_DATA;
            }
            /* FIXME - Not sure why every state that is NOT READING MEMORY fell into here */
            else if(bootloader.bootloaderState != READING_MEMORY && 
                    bootloader.bootloaderState != ERASED &&  // already handled above
                    bootloader.bootloaderState != PROGRAMMING &&  // already handled above
                    bootloader.bootloaderState != PROGRAMMING_DONE &&  // already handled above
                    bootloader.bootloaderState != BLANK_CHECKED) // already handled above
            {
                
                // Set status field in reply
                replyBuffer[BL_RPLY_STATUS_INDEX] = status; // APP START IS the only thing that gets HERE now
            }

            send_success = canSendMsg(can_id, replyBuffer);
            if(send_success != true){
                err = __LINE__;
            }
            
            can_id &= CRIS_COMMAND_MASK;
            memset(bootloader.rxBuffer, 0, MAX_CAN_DATA_BYTES);
        }
    } 

    
    
    
    if(!err){
        // Disable interrupts
        cli();

        // Disable interrupt sources
        CANGIE = 0x00;
        CANIE2 = 0x00;

        // Enable change of Interrupt Vectors 
        MCUCR = (1<<IVCE);
        // Move interrupts to application Flash section 
        MCUCR = 0x00;
        
        asm volatile("jmp 0x0000"); // Jump to application 
    }
}

/********************************************************************************
 * @brief Receive CAN message.  This function is called whenever a CAN
 * RX interrupt is triggered.  It extracts the data from the received message
 * and puts it into a buffer. 
 *
 *
 * @param rxBuffer - uint8_t array pointer
 *
 * @returns void
 */
void commandRx(uint8_t* rxBuffer)
{
    uint8_t canpage = 0;
    while(!(CANSIT & (1 << canpage)))
    {
        canpage++;
    }
    CANPAGE = (canpage << 4);
    
    for(uint8_t i = 0; i < (CANCDMOB & (0x0F)); i++)
    {
        rxBuffer[i] = CANMSG;
    }
    CANSTMOB &= ~(1 << RXOK);
}

/********************************************************************************
 * @brief Writes data to flash pages
 *
 *
 * @param programData - Byte array containing machine instructions that
 * will be written into flash
 *
 * @param page - Page address to write instructions to.  Page size for
 * ATmega64m1 is 256 bytes
 *
 * @returns void
 */
void flashPageWrite(uint8_t* programData, uint16_t page)
{
    uint16_t i;
    uint8_t sreg;
    unsigned char *buf = programData;

    sreg = SREG;
    cli();
    eeprom_busy_wait();
    boot_page_erase (page);
    boot_spm_busy_wait();
                        
    for (i = 0; i < FLASH_PAGE_SIZE_BYTES; i += WORD_SIZE_BYTES)
    {
        uint16_t w = *buf++;
        w += (*buf++) << 8;
                            
        boot_page_fill (page + i, w);
    }
                        
    boot_page_write(page);
                        
    boot_spm_busy_wait();
    boot_rww_enable ();
    SREG = sreg;
}

/********************************************************************************
 * @brief Processes rx'd commands and sets state variables/extracts 
 * function arguments from packets.
 *
 *
 * @param bootloader - pointer to bootloader struct that keeps track of
 * state variables, start/end read addresses etc.
 *
 * @returns status - variable indicating whether command was processed successfully
 * or contained valid arguments to pass to functions
 */
uint8_t commandProcess(bootloader_t* bootloader)
{
    uint8_t status = COMMAND_OK;
    switch(bootloader->bootloaderState)
    {
        case IDLE:
            if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_START_APPLICATION)
            {
                bootloader->bootloaderState = APPLICATION_START;
            }
            else if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == bootloader->nodeId)
            {
                bootloader->bootloaderState = PROGRAMMING_MODE_INITIALIZE;
                status = BL_RPLY_CLIENT_COM_OPENED;
            }
            break;
        case ERASED:
        /* State machine above needs to distinguish between erasing, erased, and program init */
        /*  This state maching just falls through the ERASED case                             */
        case PROGRAMMING_MODE_START:
        /*  Start and end address have been set but programming has not started yet           */
        /*  This state machine just falls thorugh the Program Start state                     */
        case BLANK_CHECKED:
        /*  Blank checking has been performed.  Fall through                                  */
        case PROGRAMMING_MODE_INITIALIZE:
            if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_PROG_INIT)
            {
                bootloader->programEndAddress = (bootloader->rxBuffer[3] << 8 | bootloader->rxBuffer[4]);
                bootloader->crc = *((uint16_t*)&bootloader->rxBuffer[5]);
                if(bootloader->programEndAddress >= END_APPLICATION_FLASH - 2)// -2 to leave room for 16 bit CRC
                {
                    bootloader->programEndAddress = 0;
                    status = BL_RPLY_CMD_OK_INVALID_ADDR;
                }
                else
                {
                    status = BL_RPLY_PROG_DATA_OK_MORE;
                    bootloader->bootloaderState = PROGRAMMING_MODE_START;
                }
            }
            else if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_ERASE_FLASH && 
                    bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX + 1] == 0xE0 && // Erase all application flash
                    bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX + 2] == 0x00 )
            {
                bootloader->app_version = *((uint32_t*)&bootloader->rxBuffer[BL_RPLY_INDEX_APP_VERSION]); 
                bootloader->programEndAddress = END_APPLICATION_FLASH; // END OF PROGRAM FLASH - DON'T BLOW AWAY BOOTLOADER!!!!!
                bootloader->bootloaderState = ERASING;
                status = BL_RPLY_CLIENT_COM_OPENED;
            }
            else if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_BLANK_CHECK)
            {
                bootloader->bootloaderState = BLANK_CHECKING;
                bootloader->flashReadStartAddress = (bootloader->rxBuffer[1] << 8 | bootloader->rxBuffer[2]);
                bootloader->flashReadEndAddress = (bootloader->rxBuffer[3] << 8 | bootloader->rxBuffer[4]);
                status = BL_RPLY_CLIENT_COM_OPENED;
            }
            else // After initializing programming mode and/or erasing, go in to program mode -> takes an extra
                 // cycle to avoid writing unwanted initialization packet data into program
            {
                bootloader->bootloaderState = PROGRAMMING;
            }
            break;
            
        case PROGRAMMING:
            // Intentionally left blank
            break;
            
        case PROGRAMMING_DONE:
            if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_START_APPLICATION) // Will always start at address 0x00
            {
                bootloader->bootloaderState = APPLICATION_START;
            }
            else if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_READ_DATA)
            {
                bootloader->bootloaderState = READING_MEMORY;
                bootloader->flashReadStartAddress = (bootloader->rxBuffer[1] << 8 | bootloader->rxBuffer[2]);
                bootloader->flashReadEndAddress = (bootloader->rxBuffer[3] << 8 | bootloader->rxBuffer[4]);
            }
            break;
               
        case READING_MEMORY:
            if(bootloader->rxBuffer[BL_RPLY_COMMAND_CODE_INDEX] == BL_CMD_READ_DATA)
            {
                bootloader->bootloaderState = READING_MEMORY;
            }
            break;                
                    
        default:
            status = BL_RPLY_INVALID_STATE;
            break;
    }
    return status;
}

