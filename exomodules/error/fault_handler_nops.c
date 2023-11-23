#include "fault_handler.h"

int fh_fault_handle(fh_info_t* fh_info){return 0;}
void fh_fault_handler_init(void){};
int fh_fault_config(uint8_t e_code, uint8_t new_fault_handler, fh_action_t action){return 0;}
int fh_fault_handlers_register(uint8_t fault_num, uint8_t fault_handler){return 0;};
void* fh_fault_config_get(unsigned int* size){return 0;}
int fh_fault_handlers_registry_check(uint8_t fault_num){return 0;}
