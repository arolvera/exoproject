#include <stdint.h>
#include "iacm/iacm.h"

uint32_t iacm_integrity_check(void){return 0;}
uint32_t iacm_testbits(IACM_ITEM_t item, uint32_t tbits){return 0;}
uint32_t iacm_get(IACM_ITEM_t item){return 0;}
uint32_t iacm_base_addr_get(void){return 0;}

void iacm_init(void){};
uint32_t iacm_set(IACM_ITEM_t item, uint32_t val){return 0;}
void iacm_set_from_fault(IACM_ITEM_t item, uint32_t val){}
void iacm_set_from_isr(IACM_ITEM_t item, uint32_t val){}
int iacm_setbits(IACM_ITEM_t item, uint32_t sbits){return 0;}
int iacm_resetbits(IACM_ITEM_t item, uint32_t sbits){return 0;}

//Mem corrupter commands
void iacm_corrupt_region(uint32_t region){}
uint32_t iacm_get_crpt_rr_msk(void){return 0;}
