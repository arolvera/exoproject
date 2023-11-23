//
// Created by exolab on 1/25/23.
//

#ifndef MASTER_IMAGE_CTOR_MEM_COMP_MAP_H
#define MASTER_IMAGE_CTOR_MEM_COMP_MAP_H

/* This order must match the component key map below */
typedef enum {
  MEMCOMPONENT_BOOTLOADER = -1,   //Will segfault. Vorago does not have bootloader. Only defined for sh_helper
  MEMCOMPONENT_SYSTEM_CONTROL = 0,
#if UCV_ENABLE
  MEMCOMPONENT_UCV            = 1,
#endif
  MEMCOMPONENT_MAX
} mem_component_t;

#endif//MASTER_IMAGE_CTOR_MEM_COMP_MAP_H
