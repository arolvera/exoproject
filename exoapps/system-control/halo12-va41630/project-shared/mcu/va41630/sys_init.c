// Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
//
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//  distribution, or disclosure of this software is expressly forbidden.
//
//  This Copyright notice may not be removed or modified without prior written
//  consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//  ExoTerra Corp
//  7640 S. Alkire Pl.
//  Littleton, CO 80127
//  USA
//
//  Voice:  +1 1 (720) 788-2010
//  http:   www.exoterracorp.com
//  email:  contact@exoterracorp.com
//

#include "thruster_control.h" // for app_stat
#include "flash/hal_flash.h"
#include "storage/storage_memory_layout.h"
#include "checksum.h"
#include "magic_numbers.h"

#define DECLARE_GLOBALS
#include "ext_decl_define.h"
#include "definitions.h"
#include "device.h"


#define  EXTCLK          (10000000UL)      /* XTAL minus frequency */
#define  XTAL            (100000000UL)      /* Oscillator frequency */
#define  HBO             (18500000UL)      /* Internal HBO frequency (18-22mhz) */
#define  EXTOSC          (100000000UL)

#define  EXTCLK          (10000000UL)      /* XTAL minus frequency */
#define  XTAL            (100000000UL)      /* Oscillator frequency */
#define  HBO             (18500000UL)      /* Internal HBO frequency (18-22mhz) */
#define  EXTOSC          (100000000UL)

static inline void sys_init_percepio_timer(void)
{
    //For percepio
    VOR_SYSCONFIG->TIM_CLK_ENABLE |= 1 << 11;
    /* set reset value */
    VOR_TIM11->RST_VALUE = 0xFFFFFFFF;
    /* enable timer */
    VOR_TIM11->CTRL |= (1 << 0);
}

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = HBO;// default HBO CLK

static void sys_clk_init(void)
{
    /* Initializes peripheral clocks */
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_CLKGEN | CLK_ENABLE_IOCONFIG | CLK_ENABLE_IRQ;

    /* Initializes the MCU clock, PLL will be used to generate main MCU clock */
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_CLKSEL_SYS_Msk;// force to HBO clk
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }

    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_CLK_DIV_SEL_Msk;
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }
    VOR_CLKGEN->CTRL1 &= ~CLKGEN_CTRL1_XTAL_EN_Msk;
    VOR_CLKGEN->CTRL1 |= CLKGEN_CTRL1_XTAL_N_EN_Msk;
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_REF_CLK_SEL_Msk;
    VOR_CLKGEN->CTRL0 |= 2UL << CLKGEN_CTRL0_REF_CLK_SEL_Pos;
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }

    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_PLL_PWDN_Msk;// power up PLL
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }
    __NOP();
    __NOP();

    // setup other control bits (not in testmode, no bypass, internal FB)
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_PLL_TEST_Msk;
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_PLL_BYPASS_Msk;
    VOR_CLKGEN->CTRL0 |= CLKGEN_CTRL0_PLL_INTFB_Msk;// always set this
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }

    // reset PLL
    VOR_CLKGEN->CTRL0 |= CLKGEN_CTRL0_PLL_RESET_Msk;
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }
    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_PLL_RESET_Msk;
    for(int i = 0; i < 100000; i++) {
        __NOP();
    }

    // check for lock
    if(VOR_CLKGEN->STAT & (CLKGEN_STAT_FBSLIP_Msk | CLKGEN_STAT_RFSLIP_Msk)) {
        // not locked - delay, and check again
        for(int i = 0; i < 100000; i++) {
            __NOP();
        }
        if(VOR_CLKGEN->STAT & (CLKGEN_STAT_FBSLIP_Msk | CLKGEN_STAT_RFSLIP_Msk)) {
            while(1);
            // there is a problem - stay on HBO clk and report an error condition
        }
    }

    VOR_CLKGEN->CTRL0 &= ~CLKGEN_CTRL0_CLK_DIV_SEL_Msk;
    for(uint32_t i = 0; i < 1000000UL; i++) {
        __NOP();
        __NOP();
        __NOP();
    }
    VOR_CLKGEN->CTRL0 |= 2UL << CLKGEN_CTRL0_CLKSEL_SYS_Pos;// select PLL to sysclk

    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_CLKGEN;// ensure CLKGEN is enabled

#if BUILD_PERCEPIO
    sys_init_percepio_timer();
#endif

    // Disable all clock scaling.  We're using an external clock for this project.
    SystemCoreClock = EXTOSC;
}

/**
 * Inits ebi memroy for startup script to copy .data and .bss into ebi_sram
 */
void sys_ebi_init(void)
{
    ebi_init(ebi_bus_init);
}

#include <stdio.h>

/**
 * Sets up fpu, vector table and default clk. First function called on boot.
 */
void sys_init(void)
{
    sys_clk_init();
    flash_init();
}

/**
 * Set or clear crash status to avoid boot loops of doom
 */


void sys_stat_reset(void)
{
    uint16_t crc = 0;
    volatile app_stat_t app_stat = {.app_status.magic_number = MAGIC_APP_STAT_SIG,
                                    .app_status.active_bckp_region = NOR_UPDATE_BKUP1_START_ADDR,
                                    .app_status.crash_status = SYS_CRASH_FALSE};

    for(unsigned int i = 0; i < sizeof(app_stat_t) - sizeof(app_stat.crc); i++){
        crc = update_crc_16(crc, ((uint8_t*)&app_stat)[i]);
    }
    app_stat.crc = crc;
    volatile int sector = ((((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t)) & 0x1FF0000) >> 16);
    flash_sector_erase(sector);


    for(unsigned int i = 0; i < sizeof(app_stat_t)/sizeof(uint16_t); i++){
        flash_single_write(((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t)) + i, ((uint16_t*)&app_stat)[i]);
    }
}


void sys_stat_crash_set(crash_status_t crash_status)
{
    uint16_t crc = 0;
    volatile app_stat_t app_stat = {.app_status.magic_number = MAGIC_APP_STAT_SIG};
    volatile uint16_t data = 0;
    for(unsigned int i = 0; i < sizeof(app_stat_t)/sizeof(uint16_t); i++){
        volatile uint32_t offset = ((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t));
        data = flash_single_read(offset + i);
        ((uint16_t*)&app_stat)[i] = data;
    }

    app_stat.app_status.crash_status = crash_status;
    for(unsigned int i = 0; i < sizeof(app_stat_t) - sizeof(app_stat.crc); i++){
        crc = update_crc_16(crc, ((uint8_t*)&app_stat)[i]);
    }
    app_stat.crc = crc;
    volatile int sector = ((((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t)) & 0x1FF0000) >> 16);
    flash_sector_erase(sector);


    for(unsigned int i = 0; i < sizeof(app_stat_t)/sizeof(uint16_t); i++){
         flash_single_write(((NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t)) + i, ((uint16_t*)&app_stat)[i]);
    }

    for(unsigned int i = 0; i < sizeof(app_stat_t)/sizeof(uint16_t); i++){
        volatile uint32_t offset = (NOR_APP_CONFIGURATION_ADDR - EBI_NOR_START_ADDR) / sizeof(uint16_t);
        data = flash_single_read(offset + i);
        ((uint16_t*)&app_stat)[i] = data;
    }
}