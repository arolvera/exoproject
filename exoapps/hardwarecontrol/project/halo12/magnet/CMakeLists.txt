target_include_directories(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        )

target_sources(${EXOPROJECT_TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_command_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_control_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_event_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_state_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/magnet_adc_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/magnet_timer_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/initialize/magnet_initialize_halo12.c
        )

if((NOT ${BUILD_SIMULATOR}) OR (NOT DEFINED BUILD_SIMULATOR))
    target_sources(${EXOPROJECT_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/magnet_hardware_port_halo12_va41630.c
            ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/magnet_pwm_halo12_va41630.c
            )
endif()
