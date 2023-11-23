cmake_minimum_required(VERSION 3.10)

#Include scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake-scripts/utils.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake-scripts/exoscripts.cmake)
#set toolchain folder for project config file
set(EXOPROJECT_TOOLCHAIN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake-toolchain/")

#Tell cmake to not try to internally test compiler. Will fail
set(CMAKE_C_COMPILER_WORKS TRUE CACHE INTERNAL "")
set(CMAKE_CXX_COMPILER_WORKS TRUE CACHE INTERNAL "")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "Create compile commands for sourcetrail")

#User must set EXOPROJECT_APP_TYPE and EXOPROJECT_TARGET
set(EXOPROJECT_CONFIG_FILE "" CACHE STRING "Project config")
set(EXOPROJECT_COMPILER_FILE "" CACHE STRING "Compiler config")
set(BUILD_MASTER_IMAGE_CONSTRUCTOR OFF CACHE BOOL "Enables master image constructor build")

#Dont check for compiler and config file. Even though when building mic you need a config file
if (NOT ${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    if (NOT ((EXISTS ${EXOPROJECT_CONFIG_FILE}) AND
        (EXISTS ${EXOPROJECT_COMPILER_FILE})))
        message(FATAL_ERROR "Set valid EXORPOJECT_CONFIG_FILE and EXOPROJECT_COMPILER_FILE:\n"
                "EXOPROJECT_CONFIG_FILE: ${EXOPROJECT_CONFIG_FILE}\n"
                "EXOPROJECT_COMPILER_FILE: ${EXOPROJECT_COMPILER_FILE}\n"
                )
    endif ()
endif ()

#Bring in project config file
include(${EXOPROJECT_CONFIG_FILE})

#Vars set in config Files
set(EXOPROJECT_APP_TYPE "" CACHE STRING "type of project hardware control or thruster control or ...")
set(EXOPROJECT_APP "" CACHE STRING "project name and target for project")
set(EXODRIVER_PROCESSOR "" CACHE STRING "project name and target for project")
set(EXOMOD_GAS "" CACHE STRING "sequence choice")
set(EXOMOD_COIL "" CACHE STRING "sequence  choice")
set(EXOMOD_CUSTOMER "" CACHE STRING "sequence choice")

#Check vars are set
message(STATUS "Config Vars set:\n"
        "\tEXOPROJECT_APP_TYPE: ${EXOPROJECT_APP_TYPE} \n"
        "\tEXOPROJECT_APP: ${EXOPROJECT_APP} \n"
        "\tEXODRIVER_PROCESSOR: ${EXODRIVER_PROCESSOR}\n"
        "\tEXOMOD_GAS: ${EXOMOD_GAS}\n"
        "\tEXOMOD_COIL: ${EXOMOD_COIL}\n"
        "\tEXOMOD_CUSTOMOER: ${EXOMOD_CUSTOMER}\n"
        )

#Gen Project name based off vars pass in and vars set in config
exo_generate_project_name()

project("${EXOPROJECT_TARGET}")
message(STATUS "Building ${EXOPROJECT_TARGET}")

# Convert dash to underscore
if(NOT ${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    include(${EXOPROJECT_COMPILER_FILE})
endif()
add_executable(${EXOPROJECT_TARGET})

# Change outputdir to <cmake-build-dir>/bin and set elf externsion
target_setup_binary(${EXOPROJECT_TARGET}
                    ""
                    EXO_BIN_DIR
                    )

#Add project type dir
add_subdirectory(exoapps)

foreach (_proj ${EXOPROJECT_DEPENDS})
    message(STATUS "Adding ${_proj}")
    add_subdirectory(${_proj})
endforeach ()

#Add third party libs
add_subdirectory(third-party)

#Config Thruster control common
if (${BUILD_EXOMOD})
    message(STATUS "Building exomodules")
    add_subdirectory(exomodules/)
endif ()

exo_get_extra_libs(EXTRA_LIBS)
target_link_libraries(${EXOPROJECT_TARGET} ${EXTRA_LIBS})

if (NOT ${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    #Gen .size .bin and .hex of project
    if (CMAKE_SIZE_UTIL)
        firmware_size(${EXOPROJECT_TARGET} ${EXO_BIN_DIR})
    endif ()

    generate_object(${EXOPROJECT_TARGET} ${EXO_BIN_DIR} .bin binary)
    generate_object(${EXOPROJECT_TARGET} ${EXO_BIN_DIR} .hex ihex)
endif ()