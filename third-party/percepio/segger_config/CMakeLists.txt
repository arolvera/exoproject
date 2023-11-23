target_sources(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/SEGGER_SYSVIEW_Config_FreeRTOS.c)

target_include_directories(${EXOPROJECT_TARGET} PUBLIC .)