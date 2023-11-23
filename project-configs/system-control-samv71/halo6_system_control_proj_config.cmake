#include toolchain file

message(STATUS "Building ${EXOPROJECT_TARGET}")

include(${EXOPROJECT_TOOLCHAIN_DIR}/samv71_arm_gcc_tc.cmake)

#Configs for directory walking
set(EXOPROJECT_APP_TYPE "system-control" CACHE STRING "'system-control' to build system control")
set(EXOPROJECT_APP "halo6" CACHE STRING "project name and target for project" FORCE)
set(EXODRIVER_PROCESSOR "samv71" CACHE STRING "project name and target for project" FORCE)




set(_submod_dirs health
        #client-control
        task-monitor
        bootloader-server
        msg-handler
        osal
        storage
        update
        utils
        canopen
        icm
        error
        iacm
        #trace
        )

set(_projects_deps exodrivers
        #hardwarecontrol
        )

#Enable what external libs need to be built
set(BUILD_CANOPEN ON CACHE BOOL "Enable canopen")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_FREERTOS ON CACHE BOOL "Enable freertos")


#set configs for exomodules
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS ${_submod_dirs})

set_property(GLOBAL
        PROPERTY EXOMOD_SUBDIRS
        ${_EXOMOD_SUBDIRS}
        )

add_compile_options(
        #${CMAKE_ASM_FLAGS}
        -ffunction-sections
        -fdata-sections
        -O1
        -DBUILD_CONFIG=1
        ${WARNING_FLAGS}
)

add_definitions(-DTRACE_UART -D__VFP_FP__ -D__ATSAMV71Q21B__)



set(EXOMOD_CTRL_ANODE "hv" CACHE STRING "component ctrl choice")
set(EXOMOD_CTRL_KEEPER "hv" CACHE STRING "component ctrl choice")
set(EXOMOD_CTRL_MAGNET "hv" CACHE STRING "component ctrl choice")

#Project details
set(EXOMOD_GAS  "xenon" CACHE STRING "sequence choice" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "sequence  choice" FORCE)
set(EXOMOD_CUSTOMER "tpc" CACHE STRING "sequence choice" FORCE)

#Other project type we rely on
set(EXOPROJECT_DEPENDS ${_projects_deps})

