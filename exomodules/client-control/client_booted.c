#define MODULE_NUM MODULE_NUM_CLIENT_CONTROL
#define SUBMODULE_NUM CLIENT_CONTROL_BOOTED

#include "client_booted.h"
#include "client-control/power/client_power.h"
#include "client_control.h"
#include "client_error_details.h"
#include "error/error_handler.h"
#include "health/health.h"
#include "sys/sys_timers.h"
#include "trace/trace.h"

/**
 * Defines the components that expected to boot load. The expected and booted variables
 * have the following bit definitions:
 * bit 0 = ACP valid image
 * bit 1 = ACP component process started
 * bit 2 = MVCP valid image
 * bit 3 = MVCP magnet process started
 * bit 4 = MVCP valve process started
 * bit 5 = ECPK valid image (don't care since there must be a valid image if this code is running)
 * bit 6 = ECPK keeper process started
 *
 * Expected bits will be set when components are registered. Booted bits will be set
 * when boot messages are received from the clients.
 */
static client_boot_status_t boot_status = {
    .expected = 0,
    .booted = 0,
    .state = CBS_POWERED_OFF
};

#define CLIENT_BOOT_COMP_POS 24
#define COMP_BOOT_BOOT_ERROR(__C__, __ERR__)                \
    ((__C__  & (boot_status.expected)) << CLIENT_BOOT_COMP_POS)  | \
    (__ERR__ & ( (1 << CLIENT_BOOT_COMP_POS) - 1) )

static SYSTMR client_power_on_timer;

#define KEEPER_REV_CHECK_ENABLE 0

// See the Halo 12 power/boot sequence confluence page for definitions of these masks
#define BOOT_STATUS_MASK_ACP_VALID_IMAGE            0x01
#define BOOT_STATUS_MASK_ACP_ANODE_STARTED          0x02
#define BOOT_STATUS_MASK_MVCP_VALID_IMAGE           0x04
#define BOOT_STATUS_MASK_MVCP_MAGNET_STARTED        0x08
#define BOOT_STATUS_MASK_MVCP_VALVE_STARTED         0x10
#define BOOT_STATUS_MASK_ECPK_VALID_IMAGE           0x20
#define BOOT_STATUS_MASK_ECPK_KEEPER_STARTED        0x40

/**
 * Perform all initial boot-up checks once all MCUs are online and
 * we are operational, but before we say we are in Standby
 *
 * This function SHOULD take action if anything fails.  Taking action might be
 * going back to pre-op, for example.  This function gives a pass/fail.  If
 * it passes then it is safe to got to Standby
 *
 * This is also the right place to do things like check board revs and
 * any HSI compensations.
 *
 * return 0 if all health is good and thruster can go to Standby, nonzero otherwise
 */
int client_health_bootup_check(void)
{
    int err = 0;
#if KEEPER_REV_CHECK_ENABLE
    /**
     * For older boards that do not have the correct voltage sensor.  There is a
     * routine that runs every time we transition to standby that detects the wrong
     * values and compensates for it.
     */
    ctrl_keeper_rev_check();
#endif
    return err;
}

/**
 * Start the client power up timer, if the timer expires one of the clients
 * did not boot.  The timer and boot up status are monitored every time
 * the client_service routing is called.
 */
void client_power_up_timer(void)
{
    Thruster_State = TCS_TRANISTION_STANDBY;
    if(client_power_on_timer > 0) {
        sys_timer_abort(&client_power_on_timer);
    }
    sys_timer_start(20 SECONDS, &client_power_on_timer);
    boot_status.state = CBS_BOOTING;
}

/**
 * Clients are being shut off - stop waiting for them to boot and mark the
 * thruster state as powered off
 */
void client_power_down_timer(void)
{
    Thruster_State = TCS_POWER_OFF;
    if(client_power_on_timer > 0) {
        sys_timer_abort(&client_power_on_timer);
    }
    boot_status.state = CBS_POWERED_OFF;
}

/**
 * Register device with client control boot_status struct. This is legacy from
 * Halo 6. The struct has an "expected" variable and "booted" variable. The bit
 * pattern for each of these variables is defined in Confluence. The "expected"
 * variable has its bits set when components are registered. The "booted" variable
 * has its bits set when the client devices boot up and send their boot message.
 * If booted = expected then the ECPK will move into standby state.
 *
 * @param c which component to register
 */
void client_control_register_component(component_type_t c)
{
    switch(c) {
        case COMPONENT_ANODE:
            boot_status.expected |= (BOOT_STATUS_MASK_ACP_VALID_IMAGE | BOOT_STATUS_MASK_ACP_ANODE_STARTED);
            break;
        case COMPONENT_KEEPER:
            boot_status.expected |= (BOOT_STATUS_MASK_ECPK_VALID_IMAGE | BOOT_STATUS_MASK_ECPK_KEEPER_STARTED);
            break;
        case COMPONENT_MAGNET:
        case COMPONENT_MAGNET_I:
            boot_status.expected |= (BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_MVCP_MAGNET_STARTED);
            break;
        case COMPONENT_VALVES:
            boot_status.expected |= (BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_MVCP_VALVE_STARTED);
            break;
        default:
            // do nothing for now
            break;
    }
}

/**
 * Notifies this module that a client has been powered off
 * @param c
 */
inline void client_comp_powered_down(component_type_t c, bool resetting)
{
    if(resetting) {
        Thruster_State = TCS_TRANISTION_STANDBY;
    } else {
        Thruster_State = TCS_POWER_OFF;
    }

    switch(c) {
        case COMPONENT_ANODE:
            boot_status.booted &= ~(BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_ACP_ANODE_STARTED);
            break;
        case COMPONENT_KEEPER:
            boot_status.booted &= ~(BOOT_STATUS_MASK_ECPK_VALID_IMAGE | BOOT_STATUS_MASK_ECPK_KEEPER_STARTED);
            break;
        case COMPONENT_MAGNET:
        case COMPONENT_MAGNET_I:
            boot_status.booted &= ~(BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_MVCP_MAGNET_STARTED);
            break;
        case COMPONENT_VALVES:
            boot_status.booted &= ~(BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_MVCP_VALVE_STARTED);
            break;
        default:
            // do nothing for now
            break;
    }

    if(boot_status.booted == 0) {
        health_enable(false);
    }
}


inline void client_booted_register_component(component_type_t c)
{
    switch(c) {
        case COMPONENT_ANODE:
            boot_status.booted |= (BOOT_STATUS_MASK_ACP_VALID_IMAGE | BOOT_STATUS_MASK_ACP_ANODE_STARTED);
            break;
        case COMPONENT_KEEPER:
            boot_status.booted |= (BOOT_STATUS_MASK_ECPK_VALID_IMAGE | BOOT_STATUS_MASK_ECPK_KEEPER_STARTED);
            break;
        case COMPONENT_MAGNET:
        case COMPONENT_MAGNET_I:
            boot_status.booted |= (BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_MVCP_MAGNET_STARTED);
            break;
        case COMPONENT_VALVES:
            boot_status.booted |= (BOOT_STATUS_MASK_MVCP_VALID_IMAGE | BOOT_STATUS_MASK_MVCP_VALVE_STARTED);
            break;
        default:
            // do nothing for now
            break;
    }

    TraceDbg(TrcMsgMcuCtl, "c:%x", c, 0, 0, 0, 0, 0);
    if(Thruster_Control_State != TCS_TRANISTION_STANDBY) {
        /* Something came on when we were not expecting it - that is a problem.
         * This likely means that something reset.  Got to 'error' and
         * figure it out
         *
         * Note: Thruster_State is set to TCS_TRANSITION_STANDBY/POWER_OFF in
         *       the power control module when the MCU's are powered on/off.
         *       The power on/off is controlled by NMT state transitions
         */
        uint32_t status = COMP_BOOT_BOOT_ERROR(c, BS_HSI_UNEXPECTED_BOOT);
        client_boot_err(status);

        TraceE2(TrcMsgErr2, "Unexpected boot message from:%d", c, 0, 0, 0, 0, 0);

    }else if(boot_status.booted == boot_status.expected) {
        ERROR_CLEAR(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_MCU_BOOT_ERR);
        boot_status.state = CBS_BOOTED;
        health_enable(true);
        Thruster_State = TCS_STANDBY;
        client_reset_complete();
        TraceInfo(TrcMsgAlways, "********all clients booted******", 0, 0, 0, 0, 0, 0);
    }
}

void client_boot_err(boot_status_t boot_err)
{
    /* Setting stopped state turns the MCU off - but lets keep changes to
     * global boot state local to this function */
    boot_status.state = CBS_POWERED_OFF;
    client_control_specific_detail_t d = {0};
    d.boot_status = boot_err;

    ERROR_SET(TC_EMCY_REG_MANUFACTURER, ERROR_CODE_MCU_BOOT_ERR, &d);
    health_enable(false);
}

int client_boot_timout_check(void)
{
    int err = 0;
    if(boot_status.state == CBS_BOOTING && client_power_on_timer == 0) {
        uint32_t status = COMP_BOOT_BOOT_ERROR(boot_status.booted, BS_BOOT_TIMEOUT);
        client_boot_err(status);

        TraceE3(TrcMsgErr3, "Client boot error. booted:%x expected %x",
                boot_status.booted, boot_status.expected, 0, 0, 0, 0);
        err = __LINE__;
    }
    return err;
}

uint16_t client_boot_status_get_expected(void)
{
    return boot_status.expected;
}

uint16_t client_boot_status_get_booted(void)
{
    return boot_status.booted;
}

client_boot_state_t client_boot_status_get_state(void)
{
    return boot_status.state;
}


