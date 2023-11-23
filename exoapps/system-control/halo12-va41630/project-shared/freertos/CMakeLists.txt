#Port for free rtos
set(FREERTOS_CONFIG_FILE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "")

if(${BUILD_SIMULATOR})
    set(FREERTOS_PORT GCC_POSIX CACHE STRING "")
else()
    set(FREERTOS_PORT GCC_ARM_CM4F CACHE STRING "")
endif()

target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/FreeRTOS_static.c
               )
target_include_directories(${EXOPROJECT_TARGET} PUBLIC ./)

