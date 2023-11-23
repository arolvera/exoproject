####################################################################################
# MASTER_IMAGE_CONSTRUCTOR PROJECT CONFIGURATION FILE
# This project configuration file is created to build the master image for official
# deployment. It should include system control and all hardware components.
####################################################################################



####################################################################################
# Toolchain file
####################################################################################
#include(${EXOPROJECT_TOOLCHAIN_DIR}/arm_gcc_tc.cmake)



####################################################################################
# Application and Architecture Variables
####################################################################################
set(EXOPROJECT_APP "halo12" CACHE STRING "target product" FORCE)
set(EXOPROJECT_APP_TYPE "master-image-constructor" CACHE STRING "type of project: hardware control or thruster control or ..." FORCE)
set(EXOPROJECT_APP_SUBTYPE "app" CACHE STRING "project subtype" FORCE)
set(EXODRIVER_PROCESSOR "va41630" CACHE STRING "target processor" FORCE)
set(EXOMOD_GAS "xenon" CACHE STRING "target gas setpoints" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "magnet wire material" FORCE)
set(EXOMOD_CUSTOMER "boeing" CACHE STRING "target customer setpoints" FORCE)

set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Enable verbose output from cmake builds")
set(BUILD_SIMULATOR OFF CACHE BOOL "Build x86 simulator")



####################################################################################
# Exomodule Declarations
####################################################################################
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
        #    component
        #    conditioning
        #    error
        #    iacm
        osal
        trace
        storage
        update
        #    sys
        #    sequence
        #    setting
        #    client-control
        #    utils
        )
set_property(GLOBAL
        PROPERTY EXOMOD_SUBDIRS
        ${_EXOMOD_SUBDIRS}
        )



####################################################################################
# Third-party libs enable
####################################################################################
set(BUILD_CANOPEN OFF CACHE BOOL "Enable canopen")
set(BUILD_CMSIS ON CACHE BOOL "Enable cmsis")
set(BUILD_FREERTOS OFF CACHE BOOL "Enable freertos")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_PERCEPIO OFF CACHE BOOL "Enable percepio")

link_libraries(-pthread)

#Other project type we rely on
set(EXOPROJECT_DEPENDS)