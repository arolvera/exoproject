#include "device.h"
#include "flash/hal_flash.h"

#define NOR_ADDRESS_LATCH() ({for(int i = 0; i < 10; i++){}                \
                             VOR_GPIO->BANK[4].DATAOUTRAW &= ~(1 << 12);   \
                             for(int i = 0; i < 10; i++){}                 \
                             VOR_GPIO->BANK[5].DATAOUTRAW &= ~(1 << 1);    \
                             for(int i = 0; i < 10; i++){}                 \
                             })

#define NOR_DATA_LATCH()     ({for(int i = 0; i < 10; i++){}            \
                              VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 1); \
                              for(int i = 0; i < 10; i++){}             \
                              VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12);\
                              for(int i = 0; i < 10; i++){}             \
                              })

#define NOR_DATA_LINE_CLEAR  VOR_GPIO->BANK[4].DATAOUTRAW &= ~(0x3FF);   \
                             VOR_GPIO->BANK[3].DATAOUTRAW &= ~(0xFC00);

#define NOR_ADDRESS_SET(__OFFSET__) VOR_GPIO->BANK[3].DATAOUTRAW = ((__OFFSET__ & 0x01FFC000) >> 14); \
                                    VOR_GPIO->BANK[2].DATAOUTRAW = ((__OFFSET__ & 0x00003FFF) << 2);  \
                                    NOR_ADDRESS_LATCH();

#define NOR_DATA_SET(__DATA__)                           \
({                                                       \
    NOR_DATA_LINE_CLEAR;                                 \
    VOR_GPIO->BANK[3].DATAOUTRAW |= (__DATA__ & 0xFC00); \
    VOR_GPIO->BANK[4].DATAOUTRAW |= (__DATA__ & 0x3FF);  \
    NOR_DATA_LATCH();                                    \
    while(!(VOR_GPIO->BANK[0].DATAINRAW & (1 << 6))){    \
    }                                                    \
    NOR_DATA_LINE_CLEAR;                                 \
})

#define RESET()                                 \
({                                              \
    VOR_GPIO->BANK[0].DATAOUTRAW &= ~(1 << 13); \
    for(int i = 0; i < 10000; i++){             \
    }                                           \
    VOR_GPIO->BANK[0].DATAOUTRAW |= (1 << 13);  \
})


static void nv_data_write(uint32_t offset, uint16_t data)
{
    NOR_ADDRESS_SET(offset);
    NOR_DATA_SET(data);
}

static void nv_cmd_cycles(uint8_t cmd)
{
    VOR_GPIO->BANK[4].DATAOUTRAW = 0x00000000 | (1 << 13);
    VOR_GPIO->BANK[5].DATAOUTRAW = 0x00000000;

    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 0); // OE
    VOR_GPIO->BANK[4].DATAOUTRAW &= ~(1 << 12); // CE
    VOR_GPIO->BANK[5].DATAOUTRAW &= ~(1 << 1); // WE

    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 1); // WE
    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12); // CE

    NOR_ADDRESS_SET((0x555 ^ 0x300));
    NOR_DATA_SET(0x55 << 2);

    NOR_ADDRESS_SET((0x2AA ^ 0x300));
    NOR_DATA_SET(0xAA << 2);

    NOR_ADDRESS_SET((0x555 ^ 0x300));
    NOR_DATA_SET(cmd << 2);
}

static uint32_t nv_data_status_read(void)
{
    volatile uint32_t data = 0;
    volatile uint32_t datain = 0;
    volatile uint32_t data_1 = 0;
    volatile uint32_t datain_1 = 0;

    VOR_GPIO->BANK[4].DIR &= ~0x000003FF;
    VOR_GPIO->BANK[3].DIR &= ~0x0000FC00;

    data = VOR_GPIO->BANK[3].DATAINRAW;
    datain = VOR_GPIO->BANK[4].DATAINRAW;

    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 0); // OE
    VOR_GPIO->BANK[5].DATAOUTRAW &= ~(1 << 0); // OE

    data_1 = VOR_GPIO->BANK[3].DATAINRAW;
    datain_1 = VOR_GPIO->BANK[4].DATAINRAW;

    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 0); // OE
    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12); // CE

    VOR_GPIO->BANK[4].DIR |= 0x000003FF;
    VOR_GPIO->BANK[3].DIR |= 0x0000FC00;

    /* Or all the data together to read status word info if we ever need it */
    return data << 24 | datain << 16 | data_1 << 8 | datain_1;

}

static void flash_bypass_reset(void)
{
    NOR_ADDRESS_SET((0x000));
    NOR_DATA_SET(0x09 << 2);

    NOR_ADDRESS_SET((0x000));
    NOR_DATA_SET(0x00);
}

void flash_reset(void)
{
    // Power on reset
    VOR_GPIO->BANK[0].DATAOUTRAW |= (1 << 13); // RESET#

    VOR_GPIO->BANK[0].DATAOUTRAW &= ~(1 << 13); // RESET#
    VOR_GPIO->BANK[0].DATAOUTRAW |= (1 << 13); // RESET#

    VOR_GPIO->BANK[4].DATAOUTRAW &= ~(1 << 12); // CE
    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12); // CE

    // Regular reset
    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 0); // OE
    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12); // CE
    VOR_GPIO->BANK[0].DATAOUTRAW |= (1 << 13); // RESET#

    VOR_GPIO->BANK[0].DATAOUTRAW &= ~(1 << 13); // RESET#
    VOR_GPIO->BANK[0].DATAOUTRAW |= (1 << 13); // RESET#

    VOR_GPIO->BANK[4].DATAOUTRAW &= ~(1 << 12); // CE
    VOR_GPIO->BANK[5].DATAOUTRAW &= ~(1 << 0); // OE

    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12); // CE
    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 0); // OE

    VOR_GPIO->BANK[2].DATAOUTRAW = 0x00000000;
    VOR_GPIO->BANK[4].DATAOUTRAW = 0x00000000;
    VOR_GPIO->BANK[3].DATAOUTRAW = 0x00000000;
    VOR_GPIO->BANK[5].DATAOUTRAW = 0x00000000;
}

void flash_init(void)
{
    VOR_SYSCONFIG->PERIPHERAL_CLK_ENABLE |= CLK_ENABLE_IOCONFIG | CLK_ENABLE_PORTD | CLK_ENABLE_PORTA | CLK_ENABLE_PORTE | CLK_ENABLE_PORTC | CLK_ENABLE_PORTD | CLK_ENABLE_PORTE | CLK_ENABLE_PORTF;
    VOR_SYSCONFIG->PERIPHERAL_RESET &= ~(SYSCONFIG_PERIPHERAL_RESET_IOCONFIG_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTA_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTC_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTF_Msk);
    __NOP();
    __NOP();
    VOR_SYSCONFIG->PERIPHERAL_RESET |= (SYSCONFIG_PERIPHERAL_RESET_IOCONFIG_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTA_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTC_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTD_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTE_Msk | SYSCONFIG_PERIPHERAL_RESET_PORTF_Msk);

    VOR_GPIO->BANK[0].DIR |= (1 << 13);
    VOR_GPIO->BANK[0].DIR |= (1 << 7);
    VOR_GPIO->BANK[2].DIR = 0x0000FFFF;
    VOR_GPIO->BANK[4].DIR = 0x0000FFFF;
    VOR_GPIO->BANK[3].DIR = 0x0000FFFF;
    VOR_GPIO->BANK[5].DIR = 0x00000003;

    VOR_GPIO->BANK[0].DATAOUTRAW = (1 << 13) | (1 << 7);
    VOR_GPIO->BANK[2].DATAOUTRAW = 0x00000000;
    VOR_GPIO->BANK[4].DATAOUTRAW = 0x00000000 | (1 << 12) | (1 << 13);
    VOR_GPIO->BANK[3].DATAOUTRAW = 0x00000000;
    VOR_GPIO->BANK[5].DATAOUTRAW = 0x00000000;

    flash_reset();
}

int flash_sector_blank_check(uint32_t sector)
{
    volatile uint16_t data = 0;
    int err = 0;
    for(int i = 0; i < 0xFFFF && !err; i++){
        data = flash_single_read(sector + i);
        if(data != 0xFFFF){
            err = __LINE__;
        }
    }
    return err;
}

uint16_t flash_single_read(uint32_t offset)
{
    volatile uint32_t data_high = 0;
    volatile uint32_t data_low  = 0;

    VOR_GPIO->BANK[4].DATAOUTRAW = 0x00000000 | (1 << 13);
    VOR_GPIO->BANK[5].DATAOUTRAW = 0x00000000;

    NOR_ADDRESS_SET(offset);

    VOR_GPIO->BANK[4].DIR &= ~0x000003FF;
    VOR_GPIO->BANK[3].DIR &= ~0x0000FC00;


    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 1); // WE
    for(int i = 0; i < 10; i++){}
    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 13); // CE1
    VOR_GPIO->BANK[4].DATAOUTRAW &= ~(1 << 12); // CE
    for(int i = 0; i < 10; i++){}
    VOR_GPIO->BANK[5].DATAOUTRAW &= ~(1 << 0); // OE
    for(int i = 0; i < 10; i++){}
    data_high = VOR_GPIO->BANK[3].DATAINRAW & 0xFC00;
    data_low  = (VOR_GPIO->BANK[4].DATAINRAW)  & 0x3FF;
    VOR_GPIO->BANK[5].DATAOUTRAW |= (1 << 0); // OE
    VOR_GPIO->BANK[4].DATAOUTRAW |= (1 << 12); // CE
    VOR_GPIO->BANK[5].DATAOUTRAW &= ~(1 << 1); // WE
    VOR_GPIO->BANK[4].DIR |= 0x000003FF;
    VOR_GPIO->BANK[3].DIR |= 0x0000FC00;

    return data_high | data_low;
}

int flash_sector_erase(uint16_t sector_address)
{
    int err = 0;

    if(sector_address > 0x1FF){
        err = __LINE__;
    } else {
        nv_cmd_cycles(0x1);

        //////////////////////////////////////////////////
        NOR_ADDRESS_SET((0x555 ^ 0x300));
        NOR_DATA_SET(0x55 << 2);

        //////////////////////////////////////////////////
        NOR_ADDRESS_SET((0x2AA ^ 0x300));
        NOR_DATA_SET(0xAA << 2);
        ////////////////////////////////////////////////

        NOR_ADDRESS_SET(sector_address << 16);
        NOR_DATA_SET(0x0C << 2);

        ///////////////////////////////////////////////////////
        NOR_ADDRESS_SET(sector_address << 16);

        nv_data_status_read();

        for(int i = 0; i < 1000; i++){}
    }

    return err;
}

int flash_buffer_write(uint32_t offset, uint16_t* data, int len)
{
    int err = 0;
    nv_cmd_cycles(0x04);

    for(int j = 0; j < len; j++){
        NOR_ADDRESS_SET((0x555 ^ 0x300));
        NOR_DATA_SET(0x05 << 2);
        nv_data_write(offset + j, data[j]);
    }
    flash_bypass_reset();

    RESET();

    return err;
}

int flash_single_write(uint32_t offset, uint16_t data)
{
    int err = 0;

    nv_cmd_cycles(0x5);

    nv_data_write(offset, data);

    nv_data_status_read();

    return err;
}


