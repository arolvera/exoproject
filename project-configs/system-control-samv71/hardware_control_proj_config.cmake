#include toolchain file
include(${EXOPROJECT_TOOLCHAIN_DIR}/arm_gcc_tc.cmake)

#Configs for directory walking
set(EXOPROJECT_APP_TYPE "hardwarecontrol" CACHE STRING "type of project hardware control or thruster control or ..." FORCE)
set(EXOPROJECT_APP "halo12" CACHE STRING "project name and target for project" FORCE)
set(EXODRIVER_PROCESSOR "va41630" CACHE STRING "project name and target for project" FORCE)

#set configs for exomodules
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
    osal
    utils
    )

set_property(GLOBAL
             PROPERTY EXOMOD_SUBDIRS
             ${_EXOMOD_SUBDIRS}
             )

set(EXOMOD_GAS  "krypton" CACHE STRING "sequence choice" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "sequence  choice" FORCE)
set(EXOMOD_CUSTOMER "bct" CACHE STRING "sequence choice" FORCE)

#Enable what external libs need to be built
set(BUILD_CANOPEN OFF CACHE BOOL "Enable canopen" FORCE)
set(BUILD_LIBCRC OFF CACHE BOOL "Enable libcrc" FORCE)
set(BUILD_FREERTOS OFF CACHE BOOL "Enable freertos" FORCE)

#Other project type we rely on
set(EXOPROJECT_DEPENDS
    exodrivers
    thruster-control
    )


