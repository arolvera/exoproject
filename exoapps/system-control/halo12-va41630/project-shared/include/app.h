// Copyright (C) 2023 ExoTerra Corp - All Rights Reserved
//
//  Unauthorized copying of this file, via any medium is strictly prohibited
//  Proprietary and confidential.  Any unauthorized use, duplication, transmission,
//  distribution, or disclosure of this software is expressly forbidden.
//
//  This Copyright notice may not be removed or modified without prior written
//  consent of ExoTerra Corp.  ExoTerra Corp reserves the right to modify this software without notice.
//
//  ExoTerra Corp
//  7640 S. Alkire Pl.
//  Littleton, CO 80127
//  USA
//
//  Voice:  +1 1 (720) 788-2010
//  http:   www.exoterracorp.com
//  email:  contact@exoterracorp.com

#ifndef _APP_H
#define _APP_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "error/error_handler.h"

#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
    
typedef enum{
    APP_SUBMODULE,
} APP_SUBMODULES;

#pragma pack(push)                  /* push current alignment to stack        */
#pragma pack(1)                     /* set alignment to 1 byte boundary       */
typedef struct app_update_status{
    uint32_t repair_status;
    uint32_t install_status;
    uint32_t install_error;
    uint32_t reset_reason;
}app_update_status_t;
#pragma pack(pop)                  /* restore original alignment from stack   */


#define STRINGIZE_MACRO_VALUE(__X__) GET_MACRO_VALUE(__X__)
#define GET_MACRO_VALUE(__X__)       #__X__
    
/* Software Version in "major.minor.rev" format */
#define VERSION_SOFTWARE STRINGIZE_MACRO_VALUE(MINOR_VERSION) "." \
                         STRINGIZE_MACRO_VALUE(MAJOR_VERSION) "." \
                         STRINGIZE_MACRO_VALUE(REVISION) "."


#define BUILD_CONFIG_COIL_SILVER (1 << 0)
#define BUILD_CONFIG_COIL_COPPER (1 << 1)
#define BUILD_CONFIG_KRYPTON     (1 << 2)

#ifndef BUILD_CONFIG
#define BUILD_CONFIG ( BUILD_CONFIG_COIL_SILVER )
#endif

#define BUILD_CONFIG_COIL_MASK      (BUILD_CONFIG_COIL_SILVER | BUILD_CONFIG_COIL_COPPER)
#define BUILD_CONFIG_COIL_IS_SILVER (BUILD_CONFIG & BUILD_CONFIG_COIL_SILVER)
#define BUILD_CONFIG_COIL_IS_COPPER (BUILD_CONFIG & BUILD_CONFIG_COIL_COPPER)
#define BUILD_CONFIG_IS_KRYPTON     (BUILD_CONFIG & BUILD_CONFIG_KRYPTON)

/********************** Validate Build Configuration **************************/

#if ( ((BUILD_CONFIG & BUILD_CONFIG_COIL_MASK) == BUILD_CONFIG_COIL_MASK) || \
      ((BUILD_CONFIG & BUILD_CONFIG_COIL_MASK) == 0 ) )
/* One and only one must be selected */
#error Invalid COIL Configuration.  Specify proper Copper/Silver in BUILD_CONFIG. Current Value:
#pragma message STRINGIZE_MACRO_VALUE(BUILD_CONFIG)
#endif

/********************** End Coil Configuration ********************************/


/* Hardware Version */
#define VERSION_HARDWARE "PPU_EDU"

#define VERSION_COPPER   "PPU_COP"
#define VERSION_SILVER   "PPU_SLV"
    
    
#ifndef EXTERNAL_INTERFACE_CAN
#define EXTERNAL_INTERFACE_CAN      0
#endif
#ifndef EXTERNAL_INTERFACE_SERIAL
#define EXTERNAL_INTERFACE_SERIAL   1
#endif
#ifndef EXTERNAL_INTERFACE_BOTH
#define EXTERNAL_INTERFACE_BOTH     2
#endif
    
#ifndef EXTERNAL_INTERFACE
#define EXTERNAL_INTERFACE  EXTERNAL_INTERFACE_CAN
#endif
    
#define WATCHDOG_CLEAR()                                                       \
{                                                                              \
   WDT_Clear();   /* reset the watchdog timer */                               \
   RSWDT_Clear(); /* reset the reinforced safety watchdog timer */             \
}



/**
 * @brief   Initialize the application.
 *
 * Task monitor performs the work of initialization so that multiple app types
 * can be supported without duplication. ToDo: is there really any value doing
 * it in task monitor? If so, what is there to do in this function?
 *
 * Define as weak alias for compatibility
 */
void app_init( void ) __attribute__((weak));

/**
 * @brief   Launch app tasks.
 *
 * Task monitor performs the work of launching tasks. ToDo: what is there for
 * this function to do?
 *
 * Define as weak alias for compatibility
 *
 * @param pv
 */
void app_task( void *pv) __attribute__((weak));

/**
 * @brief   Get the app error detail size.
 *
 * ToDo: what is the purpose of this function? Is it still needed?
 *
 * @return  Size of the error detail.
 */
size_t APP_error_detail_size_get(void);


#ifdef __cplusplus
}
#endif

#endif /* _APP_H */

/*******************************************************************************
 End of File
 */

