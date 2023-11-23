#include <stdbool.h>
#include <stddef.h>
#include "utils/macro_tools.h"
#include "storage/storage_memory_layout.h"
#include "storage/component_keys.h"
#include "ext_decl_define.h"
#include "storage_class.h"
#include "component_map_def.h"

extern fileops_t fops_ebi_nor;
extern fileops_t fops_fram;



#if __x86_64__
//We need two instances of sram since we copy from one sram volume to another. This is only used on x86
extern fileops_t fops_ebi_sram_1;
extern fileops_t fops_ebi_sram_2;
#define FOPS_EBI_SRAM_1 (fops_ebi_sram_1)       // ToDo: fix this
#define FOPS_EBI_SRAM_2 (fops_ebi_sram_2)
#else
extern fileops_t fops_ebi_sram;
#define FOPS_NOR (fops_ebi_nor)
#define FOPS_SRAM (fops_ebi_sram)
#endif

/* These must remain in KEY order - Alphabetical from A-Z then a-z */
component_map_t  file_map[] = {
    {UPDATE_IMAGE, {SRAM_UPDATE_START_ADDR, SRAM_UPDATE_START_ADDR + UPDATE_REGION_SIZE},
     false,    sm_erase_write, &FOPS_SRAM},
    {UPDATE_IMAGE_BKUP1, {NOR_UPDATE_BKUP1_START_ADDR, NOR_UPDATE_BKUP1_START_ADDR + UPDATE_REGION_SIZE},
        false, sm_erase_write, &FOPS_NOR},
    {UPDATE_IMAGE_BKUP2, {NOR_UPDATE_BKUP2_START_ADDR, NOR_UPDATE_BKUP2_START_ADDR + UPDATE_REGION_SIZE},
     false,    sm_erase_write, &FOPS_NOR},
    {FRAM_UPDATE_IMAGE, {UPDATE_START_ADDRESS, UPDATE_START_ADDRESS + FRAM_APP_SIZE},
            false, sm_erase_write, &fops_fram},
    {APP_CONFIGURATION, {NOR_APP_CONFIGURATION_ADDR, NOR_APP_CONFIGURATION_ADDR + APP_CONFIG_SIZE},
     false,    sm_erase_write, &FOPS_NOR},
};
const size_t FILE_MAP_SIZE = SIZEOF_ARRAY(file_map);
