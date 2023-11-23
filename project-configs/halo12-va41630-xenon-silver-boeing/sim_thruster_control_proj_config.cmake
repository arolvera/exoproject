#include toolchain file
include(${EXOPROJECT_TOOLCHAIN_DIR}/x86_gcc_tc.cmake)

#Configs for directory walking
set(EXOPROJECT_APP_TYPE "system-control" CACHE STRING "type of project hardware control or system control or bootloader"
        FORCE)
set(EXOPROJECT_APP "halo12" CACHE STRING "project name and target for project" FORCE)
set(EXODRIVER_PROCESSOR "x86" CACHE STRING "project name and target for project" FORCE)
set(EXOPROJECT_APP_SUBTYPE "system-control-emu" CACHE STRING "subproject type (test, emulator, etc..)" FORCE)

#set configs for exomodules
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
        component
        conditioning
        canopen
        error
        iacm
        osal
        health
        trace
        sys
        msg-handler
        task-monitor
        sequence
        storage
        user-setting-values
        client-control
        utils
        update
        )

set_property(GLOBAL
        PROPERTY EXOMOD_SUBDIRS
        ${_EXOMOD_SUBDIRS}
        )

add_definitions(-D__VFP_FP__ -DFREERTOS_PORT=GCC_POSIX)

set(EXOMOD_CTRL_ANODE "halo12" CACHE STRING "component ctrl choice")
set(EXOMOD_CTRL_KEEPER "halo12" CACHE STRING "component ctrl choice")
set(EXOMOD_CTRL_MAGNET "halo12" CACHE STRING "component ctrl choice")

set(EXOMOD_GAS  "xenon" CACHE STRING "sequence choice" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "sequence  choice" FORCE)
set(EXOMOD_CUSTOMER "boeing" CACHE STRING "sequence choice" FORCE)

#set number of magnets
set(EXOMOD_NUM_MAGS "one" CACHE STRING "NUmber of mags to control" FORCE)


#Enable what external libs need to be built
set(BUILD_CANOPEN ON CACHE BOOL "Enable canopen")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_FREERTOS ON CACHE BOOL "Enable freertos")
set(BUILD_PERCEPIO OFF CACHE BOOL "Enable percepio")
set(BUILD_CMSIS ON CACHE BOOL "Enable cmsis")
# variable for readability, don't turn this off when building a simulator
set(BUILD_SIMULATOR ON CACHE BOOL "Build x86 simulator")
set(SIMULATOR_USE_SOCKET ON CACHE BOOL "Use a network socket to mock CAN traffic between simulators")
if(${SIMULATOR_USE_SOCKET})
    add_definitions(-DSYSTEM_CONTROL_SOCKET_SERVER)
endif()
#Other project type we rely on
set(EXOPROJECT_DEPENDS
        exodrivers
        exoapps/hardwarecontrol
        exoapps/system-control/halo12-va41630
        )

