#include toolchain file
include(${EXOPROJECT_TOOLCHAIN_DIR}/avr_gcc_tc.cmake)

#Port for free rtos
set(FREERTOS_CONFIG_FILE_DIRECTORY ${EXOPROJECT_PATH}/common/freertos/ CACHE STRING "")
set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")

#set configs for exomodules
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
    utils
    )

set_property(GLOBAL
             PROPERTY EXOMOD_SUBDIRS
             ${_EXOMOD_SUBDIRS}
             )

#Enable what external libs need to be built
set(BUILD_CANOPEN OFF CACHE BOOL "Enable canopen" FORCE)
set(BUILD_LIBCRC OFF CACHE BOOL "Enable libcrc" FORCE)
set(BUILD_FREERTOS OFF CACHE BOOL "Enable freertos" FORCE)


