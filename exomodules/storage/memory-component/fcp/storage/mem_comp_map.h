
#ifndef _MEM_COMP_MAP_H   /* Guard against multiple inclusion */
#define _MEM_COMP_MAP_H

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* This order must match the component key map below */
typedef enum {
    MEMCOMPONENT_SYSTEM_CONTROL = 0,
    MEMCOMPONENT_ANODE          = 1,
    MEMCOMPONENT_KEEPER         = 2,
    MEMCOMPONENT_MAGNETS        = 3,
    MEMCOMPONENT_VALVES         = 4,
    RESERVED                    = 5,
    MEMCOMPONENT_BOOTLOADER     = 6,
    MEMCOMPONENT_BUCK           = 7,
#if UCV_ENABLE
    MEMCOMPONENT_UCV            = 8,
#endif
    MEMCOMPONENT_MAX
} mem_component_t;


#endif