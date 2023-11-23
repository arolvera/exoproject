// Copyright (C) 2021 ExoTerra Corp - All Rights Reserved
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
//

/******************************************************************************
* INCLUDES
******************************************************************************/

//#include "app.h"
#include "thruster_control_spec.h"
#include "app_dict.h"
#include "app_emcy.h"
#include "app_stdobj.h"

/* select application drivers: */
#include "co_can_VA41630.h"      /* CAN driver                  */
#include "co_timer_VA41630.h"    /* Timer driver                */
#include "co_nvm_VA41630.h"      /* NVM driver                  */

/* User Objects */
#include "user_object_config.h"

#include "thruster_control.h"
#include "trace/trace.h"

/******************************************************************************
* PRIVATE DEFINES
******************************************************************************/

/* Define some default values for our CANopen node: */
#define APP_NODE_ID       0x22u               /* CANopen node ID             */
#define APP_BAUDRATE      1000000u            /* CAN baudrate                */
#define APP_TMR_N         16u                 /* Number of software timers   */
#define APP_TICKS_PER_SEC 1000u               /* Timer clock frequency in Hz */
#define APP_OBJ_N         384u                /* Object dictionary max size  */

/******************************************************************************
* PRIVATE VARIABLES
******************************************************************************/

static  OD_DYN      Thruster_Control_DYN;
static  CO_OBJ      Thruster_Control_DynList[APP_OBJ_N];

/* Generic I/O Device */
#define DEVICE_TYPE 0x401u
static uint32_t device_type = DEVICE_TYPE;

static const uint32_t emcy_id = 0x80;

/* allocate global variables for runtime value of objects */
static uint8_t error_register = 0;
static uint32_t manufacturer_specific = 0;

#define EMCY_HIST_MAX 8
/* object entry variable for 0x1003:0 (number of emergency errors) */
static uint8_t standard_err_field;
/* object entry variables for 0x1003:1..x (the emergency history) */
static uint32_t err_history_table[EMCY_HIST_MAX];


/* Device Name */
#ifndef HALO_SIM
#define DEVICE_NAME "PPU"
#else
#define DEVICE_NAME "SIM"
#endif

static const char device_name[] = DEVICE_NAME;
const CO_OBJ_STR Dev_Type_StringObj  = {
    0,                              /* variable for read position     */
    (uint8_t *) &device_name[0]   /* start address of string memory */
};


static const char hw_version[] = "SILVER";
//#if BUILD_CONFIG == BUILD_CONFIG_COIL_SILVER
////VERSION_SILVER
//#elif BUILD_CONFIG == BUILD_CONFIG_COIL_COPPER
//VERSION_COPPER
//#else
//VERSION_HARDWARE
//#endif
//;

const CO_OBJ_STR VER_HW_StringObj  = {
    0,                              /* variable for read position     */
    (uint8_t *) &hw_version[0]   /* start address of string memory */
};

/* Software Version */
//static const char sw_version[] = VERSION_SOFTWARE;
static const char sw_version[] = "1234";
const CO_OBJ_STR VER_SW_StringObj  = {
    0,                              /* variable for read position     */
    (uint8_t *) &sw_version[0]   /* start address of string memory */
};

/* Each software timer needs some memory for managing
 * the lists and states of the timed action events.
 */
static CO_TMR_MEM TmrMem[APP_TMR_N];

/* Each SDO server needs memory for the segmented or
 * block transfer requests.
 */
static uint8_t SdoSrvMem[CO_SDOS_N * CO_SDO_BUF_BYTE];

/* Select the drivers for your application. For possible
 * selections, see the directory /drivers. In this example
 * we select the driver templates. You may fill them with
 * your specific hardware functionality.
 */
static struct CO_IF_DRV_T AppDriver = {
    &CanOpenDriver,
    &ATSAMV71TimerDriver,
    &ATSAMV71NvmDriver
};

/*
 * \details Set the CANopen specification for the test specific environment:
 *
 *          **constant settings:**
 *          - emergency: module 'app_emcy' with pre-defined errors:
 *            
 *          **object dictionary settings:**
 *          - 1003:0 - Emergency number of errors (local variable)
 *          - 1003:X - TS_EMCY_HIST_MAX error history fields (local variables)
 *          - 1014:0 - COB-ID EMCY message (local variable)
 * 
 * @param self pointer to dynamic object dictionary 
 * 
 */
void CreateEmcy(OD_DYN *self)
{
    uint8_t n;
    
    ODAdd(self, OBJ1003_0(&standard_err_field));
    standard_err_field = 0;
    for (n = 1; n < EMCY_HIST_MAX; n++) {
        ODAdd(self, OBJ1003_X(n, &err_history_table[n]));
        err_history_table[n] = 0;
    }
    ODAdd(self, OBJ1014_0(&emcy_id));
}

/**
 * Add an SDO Server to the Dynamic Object Dictionary
 * @param self pointer to dynamic object dictionary
 * @param srv server number (Only 1 supported for now, server 0)
 * @param request request ID (0x600 + NodeID by default)
 * @param response response ID (0x580 + NodeID by default)
 */
void CreateSDOServer(OD_DYN *self, uint8_t srv, uint32_t request, uint32_t response)
{
    ODAdd(self, OBJ12XX_0(srv, 0x02));    // Two entries
    ODAdd(self, OBJ12XX_1(srv, request));
    ODAdd(self, OBJ12XX_2(srv, response));
}

/**
 * Setup TPDO Comms
 * @param self pointer to dynamic dictionary
 * @param num TPDO COMMUNICATION PARAMETER
 * @param id COMMUNICATION PARAMETER COB-ID USED BY TPDO
 * @param type TPDO COMMUNICATION PARAMETER TRANSMISSION TYPE
 * @param inhibit TPDO COMMUNICATION PARAMETER INHIBIT TIME
 * @param evtimer TPDO COMMUNICATION PARAMETER EVENT TIMER
 */
void CreateTPDOCom(OD_DYN *self, uint8_t num, uint32_t id, uint8_t type, uint16_t inhibit, uint16_t evtimer)
{
    /*
     * Note, that the subindex 04h is missing in this array. This gab is
     * specified in the CiA DS301 for backward compatibility reasons.
     * Furthermore, the subindex 00h states the highest accessible subindex(05h)
     * and NOT the number of entries. This definition is a small pitfall when
     * building this object entry.
     */
    ODAdd(self, OBJ18XX_0(num, 0x5)); // 5 sub-entries
    ODAdd(self, OBJ18XX_0(num, id));  //Not sure how this would work without using direct acesss.
    ODAdd(self, OBJ18XX_2(num, type));
    ODAdd(self, OBJ18XX_3(num, inhibit));
    ODAdd(self, OBJ18XX_5(num, evtimer));
}

/**
 * 
 * @param self pointer to dynamic dictionary
 * @param num Constant value for TPDO number (0 to 511)
 * @param map Constant value for number of application object (1 to 8)
 * @param len Reference to 8bit value with number of mapped application objects
*          in TPDO
 */
void CreateTPDOMap(OD_DYN *self, int8_t num, uint32_t *map, uint8_t  len)
{
    uint8_t n;
    ODAdd(self, OBJ1AXX_0(num, len));    // Number of Sub-Entries
    for (n = 0; n < len; n++) {
        ODAdd(self, OBJ1AXX_N(num, n+1, map[n]));
    }
}

/**
 * Create the Dynamic Object Dictionary 
 * @return pointer to root of Object Dictionary
 */
CO_OBJ *CreateDir(void)
{
    OD_DYN *self = &Thruster_Control_DYN;
    ODInit(self, Thruster_Control_DynList, APP_OBJ_N);

    /* CANOpen OD Communication Specific Standards */
    /* Device Type -- @TODO Implement this */
    ODAdd(self, OBJ1000_0(device_type));
    /* Error register */
    ODAdd(self, OBJ1001_0(&error_register));
    /* Manufacture Specific - What can/should we store here?  */
    ODAdd(self, OBJ1002_0(&manufacturer_specific));

    /* Build the Emergency Table (Indexes 0x1003 & 0x1014 */
    CreateEmcy(self);

    /* COB-ID for SYNC messages */
    ODAdd(self, OBJ1005_0((uintptr_t)0x80));

    /* System Info */
    ODAdd(self, CO_KEY(0x1008, 0, CO_STRING | CO_OBJ____R_), CO_TSTRING, (uintptr_t)&Dev_Type_StringObj);
    ODAdd(self, CO_KEY(0x1009, 0, CO_STRING | CO_OBJ____R_), CO_TSTRING, (uintptr_t)&VER_HW_StringObj);
    ODAdd(self, CO_KEY(0x100A, 0, CO_STRING | CO_OBJ____R_), CO_TSTRING, (uintptr_t)&VER_SW_StringObj);

    /* Producer Heartbeat Time */
    ODAdd(self, OBJ1017_0(&Heartbeat_Milliseconds));

    /* Identity (?) -- TODO - what this for? */
    ODAdd(self, OBJ1018_0(4)); // Identity highest sub-index
    ODAdd(self, OBJ1018_1(0)); // Vender ID - assigned by CiA @todo - get one
    ODAdd(self, OBJ1018_2(0)); // Product Code @todo Add product code for Exoterra projects
    ODAdd(self, OBJ1018_3(0)); // Revision number @todo  implement HW rev
    ODAdd(self, OBJ1018_4(0)); // Serial number @todo  implement serial number

    /* Setup a single SDO server at the standard address Index 0x12XX */
    CreateSDOServer(self, 0, CO_COBID_SDO_REQUEST(), CO_COBID_SDO_RESPONSE());

    /* @todo implement RPDO Comms (from atmegas?)*/

    /* Setup TPDOs Index 0x18XX */
    CreateTPDOCom(self, 0, CO_COBID_TPDO_DEFAULT(0), 254, 0, 0);

    /* TPDO Mapping Parameters Index 0x1AXX @todo Better define TPDO Mappings */
    uint32_t map[3];
    map[0] = CO_KEY(0x2300, 1, 32);
    map[1] = CO_KEY(0x2300, 3, 32);
    CreateTPDOMap(self, 0, map, 3);

    /* Manufacture Specific Entries */
    user_object_init(self);

    TraceDbg(TrcMsgAlways, " Object Dictionary created: len:%d used:%d size:%d",
            self->Len, self->Used, sizeof(CO_OBJ) * APP_OBJ_N, 0,0,0);

    return self->Root;
}

/* Collect all node specification settings in a single
 * structure for initializing the node easily.
 */
struct CO_NODE_SPEC_T AppSpec = {
    APP_NODE_ID,             /* default Node-Id                */
    APP_BAUDRATE,            /* default Baudrate               */
    0,                       /* Get point to OD in init func   */
    APP_OBJ_N,               /* object dictionary max length   */
    0,                       /* Get Emcy table in init func    */
    &TmrMem[0],              /* pointer to timer memory blocks */
    APP_TMR_N,               /* number of timer memory blocks  */
    APP_TICKS_PER_SEC,       /* timer clock frequency in Hz    */
    &AppDriver,              /* select drivers for application */
    &SdoSrvMem[0]            /* SDO Transfer Buffer Memory     */
};
