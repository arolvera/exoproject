####################################################################################
# THRUSTER CONTROL PROJECT CONFIGURATION FILE
# This project configuration file is created to build system (thruster) control with
# either a fixed resistor ID (dev board or emulator) or reading the ID from GPIO.
# ToDo: what is the difference between this and master_image_constructor?
####################################################################################



####################################################################################
# Toolchain file
####################################################################################
include(${EXOPROJECT_TOOLCHAIN_DIR}/arm_gcc_tc.cmake)



####################################################################################
# Application and Architecture Variables
####################################################################################
set(EXOPROJECT_APP "halo12" CACHE STRING "target product" FORCE)
set(EXOPROJECT_APP_TYPE "bootloaders" CACHE STRING "type of project: hardware control or thruster control or ..." FORCE)
set(EXOPROJECT_APP_SUBTYPE "app" CACHE STRING "project subtype" FORCE)
set(EXODRIVER_PROCESSOR "va41630" CACHE STRING "target processor" FORCE)
set(EXOMOD_GAS "xenon" CACHE STRING "target gas setpoints" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "magnet wire material" FORCE)
set(EXOMOD_CUSTOMER "boeing" CACHE STRING "target customer setpoints" FORCE)

set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Enable verbose output from cmake builds")
set(BUILD_SIMULATOR OFF CACHE BOOL "Build x86 simulator")
set(BUILD_BOOTLOADER ON CACHE BOOL "build a bootloader")


####################################################################################
# Exomodule Declarations
####################################################################################
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
        utils
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

#Other project type we rely on
set(EXOPROJECT_DEPENDS
        exodrivers
        exoapps/system-control/halo12-va41630/project-shared/
        exoapps/hardwarecontrol/include
        exomodules/storage/include
        exomodules/storage/memory-component/halo12
        )





