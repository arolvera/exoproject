/*-----------------------------------------------------------------------------
* Name: main.c
* Purpose: main Template
* Rev.: 1.0.0
*-----------------------------------------------------------------------------*/
/* Copyright (c) 2013 - 2015 ARM LIMITED
 All rights reserved.
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 - Redistributions of source code must retain the above copyright
 notice this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 - Neither the name of ARM nor the names of its contributors may be used
 to endorse or promote products derived from this software without
 specific prior written permission.
 *
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS �AS IS�
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 ---------------------------------------------------------------------------*/
/* Include files */
#include <string.h>
#include "sys_init.h"
#include "trace/trace.h"
#include "sys/sys_timers.h"
#include "flash/hal_flash.h"
#include "serial/hal_serial.h"
#include "stdio.h"


//External uart init struct
const uart_init_t uart_ini = {
        .uart_if_id = UART_IF_ID_0, .baud = UART_BAUD_115200, .rx_irq_enable = true, .tx_irq_enable = false,
        .tx_irq_priority = 5, .rx_irq_priority = 5,
        .rx_interrupt_level = sizeof(message_t), .tx_interrupt_level = sizeof(message_t),
        };


int main(void)
{
    sys_init();
    //sys_timer_init();
    //trcInit();
    int fd = -1;
    fd = uart_init(&uart_ini);

    volatile uint16_t read_data0;
    volatile uint16_t read_data1;
    volatile uint16_t read_data2;

    flash_init();

    uint16_t arr[512] = {0};
    for(int i = 0; i < 512; i++){
        arr[i] = i;
    }

    while(1) {
        /////////////////////////////////////////////////////////

        /* Erase sector 5 */

/* Turn on to test "buffer" flash writes (faster than single writes) */
# if 0
        /* Write buffer */
        flash_buffer_write(0x50003, arr, 5);

        /* Read buffer back */
        read_data0 = flash_single_read(0x50003);
        read_data1 = flash_single_read(0x50004);
        read_data2 = flash_single_read(0x50005);
#endif
        volatile int ctr = 0;
        volatile int err_i = 0;

/* Turn on to test single flash writes */
#if 1
        int page = 0;
        flash_sector_erase(page);
        for (int j = 0; j < 7; j++) {

            for (int i = 0; i < 512; i++) {
                flash_single_write((page * 512) + i, arr[i]);
                read_data0 = flash_single_read((page * 512) + i);

                if (read_data0 != arr[i]) {
                    err_i = i;
                    ctr++;
                }
            }
            page++;
#endif

            int i = 0;
            int err_list[3] = {0};
            if (read_data0 != arr[0]) {
                err_list[i++] = ctr;
            }

            if (read_data1 != arr[1]) {
                err_list[i++] = err_i;
            }

            if (read_data2 != arr[2]) {
                err_list[i++] = __LINE__;
            }
        }
    }

    return 0;

}
