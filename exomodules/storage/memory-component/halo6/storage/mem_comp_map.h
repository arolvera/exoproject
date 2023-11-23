/* This order must match the component key map below */

#ifndef MEM_COMP_MAP_H
#define MEM_COMP_MAP_H

typedef enum {
    MEMCOMPONENT_SYSTEM_CONTROL = 0,
    MEMCOMPONENT_ANODE          = 1,
    MEMCOMPONENT_KEEPER         = 2,
    MEMCOMPONENT_MAGNETS        = 3,
    MEMCOMPONENT_VALVES         = 4,
    MEMCOMPONENT_BOOTLOADER     = 5,
#if UCV_ENABLE
    MEMCOMPONENT_UCV            = 6,
#endif
    MEMCOMPONENT_BUCK           = 7,
    MEMCOMPONENT_MAX
} mem_component_t;

#endif
