/**
* @file    main.c
*
* @brief   Main entry point for the Halo 12 production app.
*
* @copyright   Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited
* proprietary and confidential.  Any unauthorized use, duplication, transmission,
* distribution, or disclosure of this software is expressly forbidden.
*
* This Copyright notice may not be removed or modified without prior written
* consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this
* software without notice.
*/

#include "include/bootloader.h"
#include "fram/fram_va41630_cmn.h"
#include "device.h"
#include "flash/hal_flash.h"
#include "definitions.h"
#include "timer/hal_timer.h"
#include "thruster_control.h"

uint8_t msp[4] = {0};
uint8_t reset_vector[4] = {0};
int node_id = 0x22;

#define NODE_ID_MASK 0x7F
#define SERIAL_START_OF_FRAME 0xa8

typedef int (*tx_msg_impl_t)(int, message_t*);
typedef int (*rx_msg_impl_t)(int, message_t*);
typedef int (*comm_init_impl_t)(void);
typedef int (*bkp_img_install_impl_t)(void);

typedef struct bl_comm_ops{
    tx_msg_impl_t tx_msg_impl;
    rx_msg_impl_t rx_msg_impl;
    comm_init_impl_t comm_init_impl;
    bkp_img_install_impl_t bkp_img_install_impl;
}bl_comm_ops_t;

bl_comm_ops_t bl_comm;

/* CLIENT MEMBER_FUNCTIONS ***********************************************************************************/
int client_tx_msg(int handle, message_t* msg)
{
    int err = 0;
    if(msg == NULL || handle < 0 ){
        err = __LINE__;
    } else {
        can_send(handle, msg, 1000);
    }
    return err;
}

int client_rx_msg(int handle, message_t* msg)
{
    int rx_rdy = 0;
    can_rx_service(handle);
    if(!can_rx_is_empty(handle)) {
        can_rcv(handle, msg, 0);
        rx_rdy = 1;
    }
    return rx_rdy;
}

int client_comm_init(void){
    /* READ RESISTOR ID TO DERTERMINE NODE ID */
    can_rx_buffer_t can_rx_buf[] = {
            {
                    .rx_mob_count = 1,
                    .filter_type = CAN_FILTER_ID,
                    /* SDO packet IDs are 0x600 | node ID.  See canopen spec */
                    .filter_high = SEG_TFER_SVR_ID | node_id,
                    .filter_low  = SEG_TFER_SVR_ID | node_id
            }
    };

    can_init_t can_ini = {
            .baud = CAN_BAUD_RATE_1000,
            .tx_mob_count = 10,
            .rx_buffers = can_rx_buf,
            .rx_buffer_len = sizeof(can_rx_buf)/sizeof(can_rx_buf[0]),
            .irq_priority = 0
    };

    int can_handle = can_init(&can_ini);
    return can_handle;
}




int client_backup_image_install(void)
{
    /* no backup images on clients, so error out */
    return -1;
}
/* CLIENT MEMBER_FUNCTIONS ***********************************************************************************/


/* SERVER MEMBER_FUNCTIONS ***********************************************************************************/
#include "serial/uart_driver_vorago.h"

#include "flash/hal_flash.h"

//External uart init struct
const uart_init_t uart_ini = {
        .uart_if_id = UART_IF_ID_0, .baud = UART_BAUD_115200, .rx_irq_enable = false, .tx_irq_enable = false,
        .tx_irq_priority = 0, .rx_irq_priority = 0,
        .rx_interrupt_level = sizeof(message_t), .tx_interrupt_level = sizeof(message_t),
};
int server_comm_init(void)
{
    int fd = -1;
    fd = uart_init(&uart_ini);
    return fd;
}

int server_tx_msg(int handle, message_t* msg)
{
    int err = 0;
    if(msg == NULL){
    err = __LINE__;
    } else {
        serial_frame_t sf = {0};
        uint16_t temp = (msg->id << 8) & 0xFF00;
        sf.sof = ((msg->id >> 8) | temp) | 0xA8;
        sf.dlc = msg->dlc;
        memcpy(sf.data, msg->data, msg->dlc);
        uint16_t crc = 0;
        for (unsigned int i = 0; i < sizeof(sf) - sizeof(sf.crc); i++) {
            volatile uint8_t data = ((uint8_t *) &sf)[i];
            crc = update_crc_16(crc, data);
        }
        sf.crc = crc;

        uart_write_raw(handle, (uint8_t *) &sf, sizeof(sf));
    }
    return err;
}


int server_rx_msg(int handle, message_t* msg){
    int rx_rdy = 0;
    volatile serial_frame_t serial_frame = {0};
    volatile uint32_t data_len = ((VOR_UART0->STATE & (0xFF << 8)) >> 8);
    if(data_len == sizeof(serial_frame)){
        for(unsigned int i = 0; i < sizeof(serial_frame); i++){
            ((volatile uint8_t*)&serial_frame)[i] = VOR_UART0->DATA;
        }
        uint16_t temp = (serial_frame.sof << 8) & 0xFF00;
        serial_frame.sof= ((serial_frame.sof >> 8) | temp);
        msg->id = serial_frame.sof & ~(0xA8 << 8);
        memcpy(msg->data, (void*)serial_frame.data, sizeof(serial_frame.data));
        msg->dlc = serial_frame.dlc;
        rx_rdy = 1;
    } else if(data_len > sizeof(serial_frame)){ /* something bad happened, flush the receiver */
        volatile uint8_t flush_byte = 0;
        for(unsigned int i = 0; i <= data_len; i++){
            flush_byte = VOR_UART0->DATA;
        }
    }

    return rx_rdy;
}


int server_backup_image_install(void)
{
    size_t sizeof_uint16 = sizeof(uint16_t);
    uint16_t crc = 0;
    UpdateFileHeader_t uh = {0};
    /* Divide bye addresses by 2 to get word addresses for nor chip */
    uint32_t backup_addr_1 = ((NOR_UPDATE_BKUP1_START_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t));
    uint32_t backup_addr_2 = ((NOR_UPDATE_BKUP2_START_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t));
    uint32_t addr = backup_addr_1;

    app_stat_t app_stat = {0};

    for(unsigned int i = 0; i < sizeof(app_stat_t)/sizeof(uint16_t); i++) {
        ((uint16_t*)&app_stat)[i] = flash_single_read(((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t)) + i);
    }
    /* If it's the active image and we didn't just crash, update from active image */
    if(app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP1_START_ADDR &&
               app_stat.app_status.crash_status == SYS_CRASH_FALSE){
        addr = backup_addr_1;
    } else if(app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP2_START_ADDR &&
                      app_stat.app_status.crash_status == SYS_CRASH_FALSE) {
        addr = backup_addr_2;
    } else if(app_stat.app_status.crash_status == SYS_CRASH_TRUE){
        /* Else if we did just crash, update with the stale image which _shouldn't_ crash */
        addr = app_stat.app_status.active_bckp_region == NOR_UPDATE_BKUP1_START_ADDR ? backup_addr_2 : backup_addr_1;
    }

    int err = 0;
    /* Verify the header in flash */
    for(unsigned int i = 0; i < sizeof(UpdateFileHeader_t) / sizeof_uint16; i++) {
        (((uint16_t*)&uh)[i]) = flash_single_read(addr++);
    }

    for(unsigned int i = 0; i < sizeof(UpdateFileHeader_t) - sizeof_uint16; i++){
        crc = update_crc_16(crc, (((uint8_t*)&uh)[i]));
    }

    if(crc == 0 || crc != uh.crc){
        err = __LINE__;
    } else {
        /* Write the header to fram for verification */
        fram_write(UPDATE_START_ADDRESS, ((uint8_t*)&uh), sizeof(uh));
    }

    if(!err){
        /* Read image data from flash and write to fram
         *  - loop on image size/sizeof(uint16_t) because the fram chip is 16 bit, but header size is 8 bit */
        for(unsigned int i = 0; i < uh.image_hdr[DEVICE_ID].image_info.size / sizeof_uint16; i++) {
            uint16_t image_data = flash_single_read(addr++);
            fram_write(APP_START_ADDRESS + (i * sizeof_uint16), (uint8_t *) &image_data, sizeof_uint16);
        }
    }
    return err;
}


/* SERVER MEMBER_FUNCTIONS ***********************************************************************************/


static int msp_rv_validate(void)
{
    int err = 0;

    /* Read stack pointer */
    fram_read(UPDATE_START_ADDRESS + sizeof(UpdateFileHeader_t), msp, 4);
    /* Read reset vector address */
    fram_read(UPDATE_START_ADDRESS + sizeof(UpdateFileHeader_t) + 4, reset_vector, 4);

    /* Make sure stack pointer is right (for va41630) */
    if(*((uint32_t*)msp) != VA41630_STACK_POINTER){
        err = __LINE__;
    }

    if(!err){
        /* Make sure image was linked right and vector table is where we want it */
        if((*((uint32_t*)reset_vector) & APP_START_ADDRESS) != APP_START_ADDRESS){
            err = __LINE__;
        }
    }

    return err;
}

static void send_it(void)
{


    SCB->CPACR |= ((3U << 10U * 2U) | /* set CP10 Full Access */
                   (3U << 11U * 2U)); /* set CP11 Full Access */

    /* Set clkgen to hbo (default) */
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_CLKSEL_SYS_Msk; // force to HBO clk
    for(uint32_t i=0; i<500; i++){
        __NOP();
    }
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_CLK_DIV_SEL_Msk; // select 1x divide

    /* Clear all interrupts set. */
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICPR[0] = 0xFFFFFFFF;
    // Barriers
    __DSB();
    __ISB();
    /* Set the address of the app vector table */
    SCB->VTOR = (uint32_t)APP_START_ADDRESS;
    // Barriers
    __DSB();
    __ISB();

    /* Set the stack pointer */
    __set_MSP(*((uint32_t*)msp));
    /* Send it! */
    asm("bx %0"::"r" (*((uint32_t*)reset_vector)));
}

static int image_validate(bool crash_check)
{
    int err = 0;
    UpdateFileHeader_t uh = {0};

    /* Validate the header */
    fram_read(UPDATE_START_ADDRESS, (uint8_t*)&uh, sizeof(uh));
    if(uh.info.magic_number != MAGIC_NUMBER_UPDATE_HDR){
        err = __LINE__;
    } else {
        uint16_t crc = 0;
        for (uint16_t i = 0; i < uh.info.header_size - sizeof(uh.crc); i++) {
            crc = update_crc_16(crc, ((uint8_t *) &uh)[i]);
        }
        if (crc != uh.crc) {
            err = __LINE__;
        }

        /* validate stack pointer and reset vector */
        err = msp_rv_validate();

        /* Validate the image */
        if (!err) {
            // TODO Read resistor ID to determine which image we need to update, Just use device 0 for development
            uint16_t crc_match = uh.image_hdr[DEVICE_ID].crc;
            crc = 0;
            uint32_t app_addr_start = UPDATE_START_ADDRESS + sizeof(uh);
            for (uint32_t i = 0; i < uh.image_hdr[DEVICE_ID].image_info.size; i++) {
                uint8_t fram_data = 0;
                fram_read(app_addr_start + i, &fram_data, 1);
                crc = update_crc_16(crc, fram_data);
            }
            if (crc != crc_match) {
                err = __LINE__;
            }
        }

        if(!err){
            app_stat_t app_stat = {0};
            for(unsigned int i = 0; i < sizeof(app_stat_t) / sizeof(uint16_t); i++){
                volatile uint32_t offset = ((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t));
                volatile uint16_t data = flash_single_read( offset+ i);
                ((uint16_t*)&app_stat)[i] = data;
            }

            uint16_t crc_match = app_stat.crc;
            crc = 0;
            for(unsigned int i = 0; i < sizeof(app_stat_t) - sizeof(app_stat.crc); i++){
                crc = update_crc_16(crc, ((uint8_t*)&app_stat)[i]);
            }
            if(crc != crc_match){
                sys_stat_reset();
            } else if(crash_check){
                /* The app crashed.  Let's make sure we don't get into the boot loop of doom */
                if(app_stat.app_status.crash_status == SYS_CRASH_TRUE){
                    err = __LINE__;
                }
            }
        }
    }
    return err;
}


static inline int segmented_transfer_process(int handle, message_t* msg)
{
    static uint32_t addr = UPDATE_START_ADDRESS;
    static UpdateFileHeader_t uh = {0};
    static message_t client_ack = SEG_TFER_FIRST_CLIENT_ACK;
    client_ack.id |= node_id;
    static unsigned int len_rxd = 0;
    static int err = 0;
    static unsigned int expected_toggle_bit_state = 0;
    static uint32_t expected_size = 0;

    uint8_t *client_ack_cmd = &client_ack.data[0];

    /* Extract server cmd byte */
    uint8_t server_cmd = msg->data[0];

    if (server_cmd == CMD_SERVER_SEG_TFER_INIT) {
        /* The server is trying to send an image so get ready */
        err = 0;
        *client_ack_cmd = CMD_CLIENT_SEG_TFER_INIT_ACK;
        memset(&uh, 0, sizeof(uh));
        len_rxd = 0;
        addr = UPDATE_START_ADDRESS;
        bl_comm.tx_msg_impl(handle, &client_ack);
        /* Set up the client repsonse cmd for subsequent packets  */
        *client_ack_cmd = CMD_CLIENT_SEG_TFER_DATA_ACK;
        expected_size = *((uint32_t*)&msg->data[4]);
        expected_toggle_bit_state = (server_cmd & CMD_SEG_TFER_TOGGLE_BIT);
    } else {
        if ((server_cmd & CMD_SEG_TFER_TOGGLE_BIT) != expected_toggle_bit_state) {
            /* We missed a packet - no good */
            err = __LINE__;
        } else {

            /* Toggle the toggle bit */
            expected_toggle_bit_state ^= CMD_SEG_TFER_TOGGLE_BIT;

            unsigned int bytes_to_rx = 0;
            if (len_rxd < sizeof(uh)) {
                /* - Read data into temporary header to validate before doing any accesses to FRAM
                   - Keep track of bytes_to_rx to make sure we don't access beyond the struct boundary */
                bytes_to_rx = (len_rxd + SEG_TFER_DATA_LEN < sizeof(uh)) ? SEG_TFER_DATA_LEN :
                              SEG_TFER_DATA_LEN - (len_rxd + SEG_TFER_DATA_LEN - sizeof(uh));
                memcpy(&(((uint8_t *) &uh)[len_rxd]), &msg->data[1], bytes_to_rx);
                len_rxd += bytes_to_rx;

                /* header has allegedly been received, so validate it */
                if (len_rxd >= sizeof(uh)) {
                    if (err || uh.info.magic_number != MAGIC_NUMBER_UPDATE_HDR) {
                        err = __LINE__;
                    } else {
                        /* write the header to FRAM, then read it back and validate it to make sure the write
                         *  didn't get corrupted.  No YOU'RE being paranoid. */
                        fram_write(addr, (uint8_t *) &uh, sizeof(uh));
                        memset(&uh, 0, sizeof(uh));
                        fram_read(addr, (uint8_t *) &uh, sizeof(uh));
                        uint16_t crc = 0;
                        for (uint16_t i = 0; i < uh.info.header_size - sizeof(uh.crc); i++) {
                            crc = update_crc_16(crc, ((uint8_t *) &uh)[i]);
                        }
                        if (crc != uh.crc) {
                            err = __LINE__;
                        }
                    }

                    /* If header is valid, write the rest of the packet, else don't bother */
                    if (!err) {
                        addr += sizeof(uh);
                        /* - Make sure we don't access beyond the array bounds
                           - Access at bytes_to_rx + 1 to get next byte after last header byte written above */
                        if (bytes_to_rx + 1 < sizeof(msg->data)) {
                            fram_write(addr, (uint8_t *) &msg->data[bytes_to_rx + 1],
                                       SEG_TFER_DATA_LEN - bytes_to_rx);
                        }
                        len_rxd += (SEG_TFER_DATA_LEN - bytes_to_rx);
                        addr += (SEG_TFER_DATA_LEN - bytes_to_rx);
                    }
                }
                /* We're done with the header.  Now just accept full length packets.  CANopen will tell us
                 * when we're done */
            } else if (len_rxd >= sizeof(uh) && !err) {
                fram_write(addr, (uint8_t *) &msg->data[1], SEG_TFER_DATA_LEN);
                addr += SEG_TFER_DATA_LEN;
                len_rxd += SEG_TFER_DATA_LEN;

                if (len_rxd >= uh.image_hdr[DEVICE_ID].image_info.size + sizeof(uh) && len_rxd >= expected_size) {
                    if (server_cmd & CMD_SERVER_SEG_TFER_LAST_SEG_BIT) {
                        /* Verify everything, don't check for a crash */
                        err = image_validate(false);
                        if (!err) {
                            sys_stat_crash_set(SYS_CRASH_FALSE);
                            /* If all is well, reset.  System will read bootloader + new app,
                             * verify the app, then jump to it */
                            NVIC_SystemReset();
                        }
                    } else {
                        err = __LINE__;
                    }
                }
            }
            bl_comm.tx_msg_impl(handle, &client_ack);
            /* ACK the server's data packets */
            CMD_CLIENT_BIT_TOGGLE(client_ack_cmd);
        }
    }



    return err;
}

static int fd = -1;
static void bl_interrupt_callback(void)
{
    message_t msg = BOOTLOADER_NEW_IMAGE_REQUEST;
    msg.id |= node_id;
    msg.data[3] = node_id;
    bl_comm.tx_msg_impl(fd, &msg);
}



int main(void)
{
    sys_init();
    fram_init();
    flash_init();




    volatile uint16_t rid0 = (VOR_GPIO->BANK[5].DATAINRAW & (1 << 5)) >> 5; // F5
    volatile uint16_t rid1 = (VOR_GPIO->BANK[5].DATAINRAW & (1 << 4)) >> 4; // F5// F4
    volatile uint16_t rid2 = (VOR_GPIO->BANK[5].DATAINRAW & (1 << 3)) >> 3; // F5// F4
    volatile uint16_t rid3 = (VOR_GPIO->BANK[5].DATAINRAW & (1 << 2)) >> 2; // F5// F4
    volatile uint16_t rid4 = (VOR_GPIO->BANK[5].DATAINRAW & (1 << 10)) >> 10; // F5// F4


    volatile uint32_t res_id = (uint32_t)((rid4 << 4) | (rid3 << 3) | (rid2 << 2) | (rid1 << 1) | rid0);

    if(res_id == ECPK_RESID){
        node_id = COMM_ID_SYSTEM_CONTROL;
        bl_comm.comm_init_impl       = server_comm_init;
        bl_comm.rx_msg_impl          = server_rx_msg;
        bl_comm.tx_msg_impl          = server_tx_msg;
        bl_comm.bkp_img_install_impl = server_backup_image_install;
    } else {
        if(res_id == MVCP_RESID) {
            node_id = COMM_ID_MAGNET;
        } else if(res_id == ACP_RESID) {
            node_id = COMM_ID_ANODE;
        }

        bl_comm.comm_init_impl = client_comm_init;
        bl_comm.rx_msg_impl = client_rx_msg;
        bl_comm.tx_msg_impl = client_tx_msg;
        bl_comm.bkp_img_install_impl = client_backup_image_install;
    }

    timer_init_t bootloader_timer = {0};
    bootloader_timer.channel = TIMER_THREE;
    bootloader_timer.interrupt_callback = bl_interrupt_callback;
    bootloader_timer.interrupt_priority = 0;
    bootloader_timer.rst_value = 1000; //  1s
    bootloader_timer.timer_units = TIMER_MILLI;

    volatile int err = 0;

    /* Check FRAM for valid image, check for crash */
    err = image_validate(true);

    /* If there isn't a valid image in fram, check for a backup */
#if 1
    if(err) {
        err = bl_comm.bkp_img_install_impl();
        if(!err) {
            /* Validate again.  No YOU"RE being paranoid */
            err = image_validate(false);
            if (!err) {
                /* If all is well, reset.  System will read bootloader + new app,
                 * verify the app, then jump to it */
                NVIC_SystemReset();
            }
        }
    } else { /* If there's a valid image in fram, boot it */
        timer_deinit(&bootloader_timer);
        send_it(); /* Should never return */
    }
#endif


    timer_init(&bootloader_timer);
    timer_enable_interrupts(TIMER_THREE, true);

    fd = bl_comm.comm_init_impl();

    /* If we get here we need a new image, so start timer to send new image request
     * periodically*/
    timer_start(TIMER_THREE, true);



    /*else:
        we're a-bootstrappin! */
    err = 0;
    while(1){
        message_t msg = {0};
        if(bl_comm.rx_msg_impl(fd, &msg)){
            /* Disable the timer so we don't freak out the segmented transfer */
            timer_start(TIMER_THREE,  false);
            if((msg.id & NODE_ID_MASK) != (long unsigned int)node_id){
                err = __LINE__;
            } else {
                err = segmented_transfer_process(fd, &msg);
            }

            if (err) {
                message_t temp = BOOTLOADER_EMCY;
                *((uint16_t*)&temp.data[6]) = err;
                temp.id |= node_id;
                bl_comm.tx_msg_impl(fd, &temp);
                timer_start(TIMER_THREE, true);
                err = 0;
            }
        }
    }
}