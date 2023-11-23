#include <stdint.h>

/**
 * Reset the NOR Flash chip
 *
 * @param void
 *
 * @return void
 */
void flash_reset(void);


/**
 * Init GPIO pins for interaction with the NOR flash chip
 *
 * @param void
 *
 * @return void
 */
void flash_init(void);


/**
 * Check if flash sector is blank (all F's)
 *
 * @param sector to blank check
 *
 * @return 0 if sector is blank (all F's), !0 if not
 */
int flash_sector_blank_check(uint32_t sector);


/**
 * Read a single 16 bit word from an offset in Flash
 *
 * @param offset to read from
 *
 * @return word read
 */
uint16_t flash_single_read(uint32_t offset);


/**
 * Erase a sector of flash (1 sector = 0xFFFF words)
 *
 * @param sector address to erase ( 0 = sector 0, 1 = sector 1, etc...)
 *
 * @return 0 on success, !0 on error
 */
int flash_sector_erase(uint16_t sector_address);


/**
 * Write a "string" of words to flash
 *
 * @param offset to begin writing at
 * @param data buffer to write from
 * @param len length of data to write from buffer
 *
 * @return 0
 */
int flash_buffer_write(uint32_t offset, uint16_t* data, int len);


/**
 * Write a single word to flash
 *
 * @param offset to write data at
 * @param data Data word to write
 *
 * @return 0
 */
int flash_single_write(uint32_t offset, uint16_t data);