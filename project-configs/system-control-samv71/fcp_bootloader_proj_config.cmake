#include toolchain file

message(STATUS "Building ${EXOPROJECT_TARGET}")

include(${EXOPROJECT_TOOLCHAIN_DIR}/samv71_arm_gcc_tc.cmake)

#Configs for directory walking

set(EXOPROJECT_APP_TYPE "bootloaders" CACHE STRING "'bootloaders' to build bootloader, 'system-control' to build system control")
set(EXOPROJECT_APP "fcp" CACHE STRING "project name and target for project" FORCE)
set(EXODRIVER_PROCESSOR "samv71" CACHE STRING "project name and target for project" FORCE)



set(_submod_dirs osal
        storage
        update
        utils)
# for error_codes.h
set(_projects_deps "${CMAKE_CURRENT_SOURCE_DIR}/exoapps/system-control/${EXOPROJECT_APP}-${EXODRIVER_PROCESSOR}")

#Enable what external libs need to be built
set(BUILD_CANOPEN OFF CACHE BOOL "Enable canopen")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_FREERTOS OFF CACHE BOOL "Enable freertos")


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

add_definitions(-DTRACE_UART -D__VFP_FP__ -D__ATSAMV71Q21B__ -DSYSTEM_CONTROL_EXEC_HDR=FLIGHT_CONTROL_EXEC_HDR_1 -DCOMP_REGION=FLIGHT_CONTROL_1)



set(EXOMOD_CTRL_ANODE "hv" CACHE STRING "component ctrl choice")
set(EXOMOD_CTRL_KEEPER "hv" CACHE STRING "component ctrl choice")
set(EXOMOD_CTRL_MAGNET "hv" CACHE STRING "component ctrl choice")

#Project details
set(EXOMOD_GAS  "na" CACHE STRING "sequence choice" FORCE)
set(EXOMOD_COIL "na" CACHE STRING "sequence  choice" FORCE)
set(EXOMOD_CUSTOMER "tpc" CACHE STRING "sequence choice" FORCE)

#Other project type we rely on
set(EXOPROJECT_DEPENDS ${_projects_deps})

