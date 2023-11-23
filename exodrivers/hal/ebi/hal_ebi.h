//
// Created by marvin on 7/19/23.
//

#ifndef SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_EXODRIVERS_HAL_EBI_HAL_EBI_H_
#define SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_EXODRIVERS_HAL_EBI_HAL_EBI_H_
#include "hal.h"

typedef struct
{
  //Sets bits [23:16] of start and end addr
  uint32_t start_addr;
  uint32_t end_addr; //Inclusive
  //Number of cycle devices needs for read or write. Experiment to get vals.
  uint8_t read_cycles;
  uint8_t write_cycles;
  uint8_t turn_around_cycles;
  uint8_t en16bitmode;
}ebi_init_t;

void ebi_init(const ebi_init_t *ebi_init_arr);
int ebi_write(uint32_t addr, uint32_t data);
uint32_t ebi_read(uint32_t addr);
void ebi_deinit(void);

#endif //SYSTEM_CONTROL_HALO12_VA41630_XENON_SILVER_BOEING_EXODRIVERS_HAL_EBI_HAL_EBI_H_
