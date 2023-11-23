####################################################################################
# CAN BM PROJECT CONFIGURATION FILE
# This project configuration file is created to ...
####################################################################################



####################################################################################
# Toolchain file
####################################################################################



####################################################################################
# Application and Architecture Variables
####################################################################################
set(EXOPROJECT_APP "halo12" CACHE STRING "project name and target for project" FORCE)
set(EXOPROJECT_APP_TYPE "system-control" CACHE STRING "type of project hardware control or thruster control or ..." FORCE)
set(EXOPROJECT_APP_SUBTYPE "storage-memory" CACHE STRING "type of project hardware control or thruster control or ..." FORCE)
set(EXODRIVER_PROCESSOR "x86" CACHE STRING "project name and target for project" FORCE)
set(EXOMOD_GAS  "xenon" CACHE STRING "target gas setpoints" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "magnet wire material" FORCE)
set(EXOMOD_CUSTOMER "boeing" CACHE STRING "target customer setpoints" FORCE)

set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Enable verbose output from cmake builds")
set(SINGLE_COMPONENT_BUILD ON CACHE BOOL "Build only the hardware ctrl component without sys ctrl?")
set(BUILD_SIMULATOR ON CACHE BOOL "Build x86 simulator")



####################################################################################
# Exomodule Declarations
####################################################################################
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
    error
    iacm
    icm
    osal
    storage
    sys
    trace
    update
    utils
    )
set_property(GLOBAL
             PROPERTY EXOMOD_SUBDIRS
             ${_EXOMOD_SUBDIRS}
             )


####################################################################################
# Third-party libs enable
####################################################################################
#Third-party libs enable
set(BUILD_CANOPEN ON CACHE BOOL "Enable canopen")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_FREERTOS ON CACHE BOOL "Enable freertos")
set(BUILD_PERCEPIO OFF CACHE BOOL "Enable percepio")
set(BUILD_CMSIS ON CACHE BOOL "Enable cmsis")

#Other project type we rely on
set(EXOPROJECT_DEPENDS
    exodrivers
    exoapps/system-control/halo12-va41630
    )



####################################################################################
# Resistor ID Settings
####################################################################################
set(EXORUN_WITH_FIXED_RESID OFF CACHE BOOL "Run the app with a fixed resistor ID for testing")
if(EXORUN_WITH_FIXED_RESID)
    add_compile_definitions(EXORUN_WITH_FIXED_RESID)

    # Turn on just one. If more than one is on, the first one will set the personality
    set(EXORUN_WITH_ECPK_RESID OFF CACHE BOOL "Run the Engine Control + Keeper mode")
    set(EXORUN_WITH_MVCP_RESID OFF CACHE BOOL "Run the Magnet + Valve mode")
    set(EXORUN_WITH_ACP_RESID OFF CACHE BOOL "Run the Anode mode")
    set(EXORUN_WITH_SYS_CTRL_RESID OFF CACHE BOOL "Run the Engine Control by itself")
    set(EXORUN_WITH_HRD_CTRL_RESID OFF CACHE BOOL "Run the Hardware Control by itself")
    set(EXORUN_WITH_ALL_CTRL_RESID OFF CACHE BOOL "Run the Engine Control + all hardware control components")

    if(EXORUN_WITH_ECPK_RESID)
        add_compile_definitions(EXORUN_WITH_ECPK_RESID)
    elseif(EXORUN_WITH_MVCP_RESID)
        add_compile_definitions(EXORUN_WITH_MVCP_RESID)
    elseif(EXORUN_WITH_ACP_RESID)
        add_compile_definitions(EXORUN_WITH_ACP_RESID)
    elseif(EXORUN_WITH_SYS_CTRL_RESID)
        add_compile_definitions(EXORUN_WITH_SYS_CTRL_RESID)
    elseif(EXORUN_WITH_HRD_CTRL_RESID)
        add_compile_definitions(EXORUN_WITH_HRD_CTRL_RESID)
    elseif(EXORUN_WITH_ALL_CTRL_RESID)
        add_compile_definitions(EXORUN_WITH_ALL_CTRL_RESID)
    endif()
endif()



####################################################################################
# Single Component Settings
####################################################################################
set(SINGLE_COMPONENT_BUILD ON CACHE BOOL "Build only one hardware ctrl component without sys ctrl")
if(SINGLE_COMPONENT_BUILD)
    add_compile_definitions(SINGLE_COMPONENT_BUILD)

    # Turn on just one. If more than one is on, the first one will set the personality
    set(EXORUN_SINGLE_COMPONENT_ANODE OFF CACHE BOOL "Run the Anode Control by itself")
    set(EXORUN_SINGLE_COMPONENT_KEEPER OFF CACHE BOOL "Run the Keeper by itself")
    set(EXORUN_SINGLE_COMPONENT_MAGNET OFF CACHE BOOL "Run the Magnet by itself")
    set(EXORUN_SINGLE_COMPONENT_VALVE OFF CACHE BOOL "Run the Valves by itself")

    if(EXORUN_SINGLE_COMPONENT_ANODE)
        add_compile_definitions(EXORUN_SINGLE_COMPONENT_ANODE)
    elseif(EXORUN_SINGLE_COMPONENT_KEEPER)
        add_compile_definitions(EXORUN_SINGLE_COMPONENT_KEEPER)
    elseif(EXORUN_SINGLE_COMPONENT_MAGNET)
        add_compile_definitions(EXORUN_SINGLE_COMPONENT_MAGNET)
    elseif(EXORUN_SINGLE_COMPONENT_VALVE)
        add_compile_definitions(EXORUN_SINGLE_COMPONENT_VALVE)
    endif()
endif()

