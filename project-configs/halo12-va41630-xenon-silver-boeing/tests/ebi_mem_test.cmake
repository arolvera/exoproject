#include toolchain file
include(${EXOPROJECT_TOOLCHAIN_DIR}/arm_gcc_tc.cmake)

#Configs for directory walking
set(EXOPROJECT_APP_TYPE "system-control" CACHE STRING "type of project hardware control or thruster control or ..." FORCE)
set(EXOPROJECT_APP_SUBTYPE "ebi-memory-test" CACHE STRING "type of project hardware control or thruster control
or ..."
    FORCE)
set(EXOPROJECT_APP "halo12" CACHE STRING "project name and target for project" FORCE)
set(EXODRIVER_PROCESSOR "va41630" CACHE STRING "project name and target for project" FORCE)

set(SINGLE_COMPONENT_BUILD 1)

add_definitions(-DTRACE_UART=1)

#set configs for exomodules
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
    utils
    trace
    osal
    )

set_property(GLOBAL
             PROPERTY EXOMOD_SUBDIRS
             ${_EXOMOD_SUBDIRS}
             )

set(EXOMOD_GAS  "xenon" CACHE STRING "sequence choice" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "sequence  choice" FORCE)
set(EXOMOD_CUSTOMER "boeing" CACHE STRING "sequence choice" FORCE)

#Third-party libs enable
set(BUILD_CANOPEN OFF CACHE BOOL "Enable canopen")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_FREERTOS OFF CACHE BOOL "Enable freertos")
set(BUILD_PERCEPIO OFF CACHE BOOL "Enable percepio")
set(BUILD_CMSIS ON CACHE BOOL "Enable cmsis")

#Other project type we rely on
set(EXOPROJECT_DEPENDS
    exodrivers
    )


