#include <stdbool.h>
#include "fram_va41630_cmn.h"
#include "device.h"

/* Commands */
#define FRAM_WREN       0x06
#define FRAM_WRDI       0x04
#define FRAM_RDSR       0x05
#define FRAM_WRSR       0x01
#define FRAM_READ       0x03
#define FRAM_WRITE      0x02
#define FRAM_RDID       0x9F
#define FRAM_SLEEP      0xB9

/* Address Masks */
#define ADDR_MSB_MASK   (uint32_t)0xFF0000
#define ADDR_MID_MASK   (uint32_t)0x00FF00
#define ADDR_LSB_MASK   (uint32_t)0x0000FF
#define MSB_ADDR_BYTE(addr)   ((uint8_t)((addr & ADDR_MSB_MASK)>>16))
#define MID_ADDR_BYTE(addr)   ((uint8_t)((addr & ADDR_MID_MASK)>>8))
#define LSB_ADDR_BYTE(addr)   ((uint8_t)(addr & ADDR_LSB_MASK))

#ifndef USE_HAL_DRIVER
#define HAL_SPI_VERSION     (0x00000300) /* 0.3.0 */

#define NUM_SPI_BANKS       (4)

#define SPI_MASTER_MSK      (0xF) /* SPI 0-3 support master mode */
#define SPI_SLAVE_MSK       (0x7) /* SPI 0-2 support slave mode */

#define SPI0_BANK           (0)
#define SPI1_BANK           (1)
#define SPI2_BANK           (2)
#define SPI3_BANK           (3)

#define SPI_CLK  (SystemCoreClock/2) /* SPI peripherals reside on APB1 */

#define SPI_MIN_WORDLEN     (4)
#define SPI_MAX_WORDLEN     (16)
#endif

#define WDFEED() VOR_WATCH_DOG->WDOGINTCLR = 1

#define FRAM_SPI_BANK 3
#define FRAM_SPI_CS   0

static void wait_idle(void);
static void delayUs(uint32_t wait);

/*******************************************************************************
 **
 ** @brief  Wait for the SPI to be idle, then clear the FIFOs
 **
 ******************************************************************************/
static void wait_idle(void)
{
    while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_TFE_Msk)) {};    /* Wait until TxBuf sends all */
    while(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS
        & SPI_STATUS_BUSY_Msk) {};    /* Wait here until bytes are fully transmitted */
    VOR_SPI->BANK[FRAM_SPI_BANK].FIFO_CLR =
        SPI_FIFO_CLR_RXFIFO_Msk | SPI_FIFO_CLR_TXFIFO_Msk;    /* Clear Tx & RX fifo */
}

/*******************************************************************************
 **
 ** @brief  Delay a specified number of us (approximate) - for FRAM wakeup time
 **
 ******************************************************************************/

static void delayUs(uint32_t us)
{
    while(us) {
        for(volatile uint32_t i = 0; i < 20; i++) {
            __ASM("NOP");
        } /* pause at least 1us at 100MHz */
        us--;
    }
}

/*******************************************************************************
 **
 ** @brief  Init a SPI periph for F-ram access (SPI in simple polling mode)
 **
 ******************************************************************************/
void fram_init(void)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= (CLK_ENABLE_SPI0 << FRAM_SPI_BANK);
    VOR_SPI->BANK[FRAM_SPI_BANK].CLKPRESCALE = 0x4;
    VOR_SPI->BANK[FRAM_SPI_BANK].CTRL0 = 0x7;
    VOR_SPI->BANK[FRAM_SPI_BANK].CTRL1 = 0x382 | (FRAM_SPI_CS << SPI_CTRL1_SS_Pos);

    /* Clear Block Protection bits to enable programming */
    /* Does not set SRWD, so WP_n pin has no effect */
    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA =
        (uint32_t)FRAM_WREN | SPI_DATA_BMSTART_BMSTOP_Msk; /* Set Write Enable Latch(WEL) bit  */
    wait_idle();
    delayUs(1000);
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA =
        (uint32_t)FRAM_WREN | SPI_DATA_BMSTART_BMSTOP_Msk; /* Set Write Enable Latch(WEL) bit  */
    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = FRAM_WRSR;    /* Write single-byte Status Register message */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA =
        (uint32_t)0x00 | SPI_DATA_BMSTART_BMSTOP_Msk;    /* Clear the BP1/BP0 protection */
    wait_idle();

}

/*******************************************************************************
 **
 ** @brief  Write to F-ram on spi[FRAM_SPI_BANK]
 **
 ******************************************************************************/
 volatile uint8_t data_dbg;
 volatile uint8_t page_buffer[512];
 volatile int start_addr = 0;
void fram_write(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t volatile voidRead __attribute__((unused));

    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA =
        (uint32_t)FRAM_WREN | SPI_DATA_BMSTART_BMSTOP_Msk; /* Set Write Enable Latch(WEL) bit  */
    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = FRAM_WRITE; /* Write command  */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MSB_ADDR_BYTE(addr); /* Address high byte */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MID_ADDR_BYTE(addr); /* Address mid byte  */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = LSB_ADDR_BYTE(addr); /* Address low byte */

    while(len - 1) {
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_TNF_Msk)) {};
        data_dbg = *buf++;
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = data_dbg;
        voidRead = VOR_SPI->BANK[FRAM_SPI_BANK].DATA;
        --len;
    }

    while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_TNF_Msk)) {}
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = (uint8_t)(*buf) | SPI_DATA_BMSTART_BMSTOP_Msk;
    wait_idle();
}

/*******************************************************************************
 **
 ** @brief  Read from F-ram on spi[FRAM_SPI_BANK]
 **
 ******************************************************************************/
uint32_t fram_read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    volatile uint32_t rb;
    uint32_t i;

    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = FRAM_READ; // Read command
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MSB_ADDR_BYTE(addr); // Address high byte
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MID_ADDR_BYTE(addr); // Address mid byte
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = LSB_ADDR_BYTE(addr); // Address low byte

    for(i = 0; i < 4; i++) {
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; // Pump the SPI
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {};
        rb = VOR_SPI->BANK[FRAM_SPI_BANK].DATA; // Void read
    }

    for(i = 0; i < len; i++) {
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; // Pump the SPI
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {}
        *buf = VOR_SPI->BANK[FRAM_SPI_BANK].DATA;
        buf++;
    }
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = SPI_DATA_BMSTART_BMSTOP_Msk; // Terminate Block Transfer
    wait_idle();
    return rb;
}

/*******************************************************************************
 **
 ** @brief  Read from F-ram on spi[FRAM_SPI_BANK]
 **
 ******************************************************************************/
void fram_read16(uint32_t addr, uint16_t *buf, uint32_t len)
{

    uint32_t volatile voidRead __attribute__((unused));
    uint32_t i;
    uint16_t tmp;

    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = FRAM_READ; /* Read command  */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MSB_ADDR_BYTE(addr); /* Address high byte */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MID_ADDR_BYTE(addr); /* Address mid byte  */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = LSB_ADDR_BYTE(addr); /* Address low byte */

    for(i = 0; i < 4; i++) {
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; /* Pump the SPI */
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {};
        voidRead = VOR_SPI->BANK[FRAM_SPI_BANK].DATA; /* Void read */
    }

    if((len % 2) > 0) { len++; } /* make len even */
    len /= 2;
    for(i = 0; i < len; i++) {
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; /* Pump the SPI */
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {}
        tmp = VOR_SPI->BANK[FRAM_SPI_BANK].DATA;
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; /* Pump the SPI */
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {}
        tmp |= ((uint16_t)((VOR_SPI->BANK[FRAM_SPI_BANK].DATA) & 0xff)) << 8;
        page_buffer[i++] = tmp;
        //buf++;
        WDFEED();
    }
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = SPI_DATA_BMSTART_BMSTOP_Msk; /* Terminate Block Transfer */
    wait_idle();

}

/*******************************************************************************
**
** @brief  Reads a section from FRAM and verifys against a buffer for match
**
** @return uint32_t if return value = addr + len, verify ok. Else, returns first
**                  failing address
**
******************************************************************************/
uint32_t fram_verify(uint32_t addr, uint8_t *buf, uint32_t len)
{
    uint32_t i;
    unsigned char readVal;

    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = FRAM_READ; /* Read command  */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MSB_ADDR_BYTE(addr); /* Address high byte */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = MID_ADDR_BYTE(addr); /* Address mid byte  */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = LSB_ADDR_BYTE(addr); /* Address low byte */

    for(i = 0; i < 4; i++) {
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; /* Pump the SPI */
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {};
        readVal = VOR_SPI->BANK[FRAM_SPI_BANK].DATA; /* Void read */
    }

    for(i = 0; i < len; i++) {
        VOR_SPI->BANK[FRAM_SPI_BANK].DATA = 0x00; /* Pump the SPI */
        while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {};
        readVal = VOR_SPI->BANK[FRAM_SPI_BANK].DATA;
        if(*buf != readVal) {
            while(!(VOR_SPI->BANK[FRAM_SPI_BANK].STATUS & SPI_STATUS_RNE_Msk)) {};
            return ((i << 24) | (*buf << 16) | (readVal << 8) | (VOR_SPI->BANK[FRAM_SPI_BANK].DATA));
        }
        buf++;
    }
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = SPI_DATA_BMSTART_BMSTOP_Msk; /* Terminate Block Transfer */
    wait_idle();

    return (addr + len);
}

/*******************************************************************************
 **
 ** @brief  Un-init the F-ram and SPI
 **
 ******************************************************************************/
void fram_sleep(void)
{

    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA =
        (uint32_t)FRAM_WREN | SPI_DATA_BMSTART_BMSTOP_Msk; /* Set Write Enable Latch(WEL) bit  */
    wait_idle();
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = FRAM_WRSR;    /* Write single-byte Status Register message */
    VOR_SPI->BANK[FRAM_SPI_BANK].DATA =
        (uint32_t)0xfd | SPI_DATA_BMSTART_BMSTOP_Msk;    /* Set the BP1/BP0 protection */
    wait_idle();

    VOR_SPI->BANK[FRAM_SPI_BANK].DATA = (uint32_t)FRAM_SLEEP | SPI_DATA_BMSTART_BMSTOP_Msk; /* Set sleep mode */
    wait_idle();

    VOR_SPI->BANK[FRAM_SPI_BANK].CTRL1 = 0;
    VOR_SPI->BANK[FRAM_SPI_BANK].CTRL0 = 0;
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE &= ~(CLK_ENABLE_SPI0 << FRAM_SPI_BANK);
}

