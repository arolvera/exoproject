#ifndef SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_EXODRIVERS_DRIVERS_VA41630_DRIVER_COMMON_FRAM_FRAM_VA41630_CMN_H_
#define SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_EXODRIVERS_DRIVERS_VA41630_DRIVER_COMMON_FRAM_FRAM_VA41630_CMN_H_
#include <stdint.h>

#define FRAM_PAGE_SIZE (1UL) //There is no concept of page but storage mod relies on page sizes.


//Sleep driver after use. Reinit on use
/* Strongly recommended in radiation environment */

void fram_init(void);
void fram_write(uint32_t addr, uint8_t *buf, uint32_t len);
uint32_t fram_read(uint32_t addr, uint8_t *buf, uint32_t len);
void fram_read16(uint32_t addr, uint16_t *buf, uint32_t len);
uint32_t fram_verify(uint32_t addr, uint8_t *buf, uint32_t len);
void fram_sleep(void);

#endif //SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_EXODRIVERS_DRIVERS_VA41630_DRIVER_COMMON_FRAM_FRAM_VA41630_CMN_H_
