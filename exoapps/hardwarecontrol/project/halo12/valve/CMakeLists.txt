target_include_directories(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        )

target_sources(${EXOPROJECT_TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_command_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_control_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_event_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_state_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/valve_adc_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/valve_timer_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/initialize/valve_initialize_halo12.c
        )

if((NOT ${BUILD_SIMULATOR}) OR (NOT DEFINED BUILD_SIMULATOR))
    target_sources(${EXOPROJECT_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/valve_hardware_port_halo12_va41630.c
            ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/valve_pwm_halo12_va41630.c
            )
endif()
