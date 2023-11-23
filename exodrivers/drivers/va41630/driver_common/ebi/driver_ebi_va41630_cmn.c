#include "ebi/hal_ebi.h"
#include "device.h"

#define NUM_EBI_BUS 4

/**
 * Initialize ebi all four ebi bus.
 * @params: ebi_init_arr - an array of 4 ebi_init_t
 * @return
 */
void ebi_init(const ebi_init_t *ebi_init_arr)
{
    uint32_t start_addr;
    uint32_t end_addr;

    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_EBI | CLK_ENABLE_IOCONFIG | CLK_ENABLE_PORTD | CLK_ENABLE_PORTA | CLK_ENABLE_PORTE | CLK_ENABLE_PORTC | CLK_ENABLE_PORTD | CLK_ENABLE_PORTE | CLK_ENABLE_PORTF;
    VOR_SYSCONFIG->PERIPHERAL_RESET &= ~(SYSCONFIG_PERIPHERAL_RESET_EBI_Msk | SYSCONFIG_PERIPHERAL_RESET_IOCONFIG_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTA_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTC_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTF_Msk);
    __NOP();
    __NOP();
    VOR_SYSCONFIG->PERIPHERAL_RESET |= (SYSCONFIG_PERIPHERAL_RESET_EBI_Msk | SYSCONFIG_PERIPHERAL_RESET_IOCONFIG_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTA_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTC_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTF_Msk);

    //Init ebi bus one at a time.
    for(int i = 0; i < NUM_EBI_BUS; i++) {
        //Addr needs to be manipulated into the ebi bus high and low for ctrl
        start_addr = (ebi_init_arr[i].start_addr >> 16) & 0xFF;
        end_addr   = (ebi_init_arr[i].end_addr   >> 16) & 0xFF;

        ((uint32_t *) &VOR_SYSCONFIG->EBI_CFG0)[i] =
            (start_addr << SYSCONFIG_EBI_CFG0_ADDRLOW0_Pos)
            | (end_addr << SYSCONFIG_EBI_CFG0_ADDRHIGH0_Pos)
            | (ebi_init_arr[i].read_cycles << SYSCONFIG_EBI_CFG0_CFGREADCYCLE_Pos)
            | (ebi_init_arr[i].write_cycles << SYSCONFIG_EBI_CFG0_CFGWRITECYCLE_Pos)
            | (ebi_init_arr[i].turn_around_cycles << SYSCONFIG_EBI_CFG0_CFGTURNAROUNDCYCLE_Pos)
            | ((ebi_init_arr[i].en16bitmode & 1) << SYSCONFIG_EBI_CFG0_CFGSIZE_Pos); //16 bit mode
    }

    VOR_GPIO->BANK[0].DATAOUTRAW |= (1 << 5);
}


void ebi_deinit(void)
{
    //Init ebi bus one at a time.
    for(int i = 0; i < NUM_EBI_BUS; i++) {
        ((uint32_t *) &VOR_SYSCONFIG->EBI_CFG0)[i] =
                (0xff << SYSCONFIG_EBI_CFG0_ADDRLOW0_Pos)
                | (0xff << SYSCONFIG_EBI_CFG0_ADDRHIGH0_Pos)
                | (0 << SYSCONFIG_EBI_CFG0_CFGREADCYCLE_Pos)
                | (0 << SYSCONFIG_EBI_CFG0_CFGWRITECYCLE_Pos)
                | (0 << SYSCONFIG_EBI_CFG0_CFGTURNAROUNDCYCLE_Pos)
                | (0 << SYSCONFIG_EBI_CFG0_CFGSIZE_Pos); //16 bit mode
    }
}

/*******************************************************************************
 **
 ** @brief $ebiwrite - Write a 32 bit word to EBI
 **
 ** @note  Requires EBI / ETH board installed to run correctly
 **
 ******************************************************************************/
int ebi_write(uint32_t addr, uint32_t data)
{
    if((addr < 0x60000000) || (addr >= 0x61000000)) {
        if((addr < 0x10000000) || (addr >= 0x11000000)) {
            return -1;
        }
    }
    *(uint32_t *)addr = data;
    return 0;
}

/*******************************************************************************
 **
 ** @brief $ebiread - Read a 32 bit word from EBI
 **
 ** @note  Requires EBI / ETH board installed to run correctly
 **
 ******************************************************************************/
uint32_t ebi_read(uint32_t addr)
{
    if((addr < 0x60000000) || (addr >= 0x61000000)) {
        if((addr < 0x10000000) || (addr >= 0x11000000)) {
            return -1;
        }
    }
    uint32_t data = *(uint32_t *)addr;
    //    txStr(txBuffer);
    return data;
}
