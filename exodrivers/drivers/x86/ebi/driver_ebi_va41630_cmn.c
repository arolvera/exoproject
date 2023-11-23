#include <fcntl.h>
#include "ebi/hal_ebi.h"

/**
 * Initialize ebi all four ebi bus.
 * @params: ebi_init_arr - an array of 4 ebi_init_t
 * @return
 */
 int fd;
void ebi_init(const ebi_init_t *ebi_init_arr)
{
    fd = open("flash", O_RDWR | O_CREAT, 0x777);
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
