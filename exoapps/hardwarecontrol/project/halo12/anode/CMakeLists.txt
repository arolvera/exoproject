target_sources(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_command_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_control_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_event_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_state_handlers_halo12.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/anode_adc_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/anode_timer_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/initialize/anode_initialize_halo12.c
        )

if((NOT ${BUILD_SIMULATOR}) OR (NOT DEFINED BUILD_SIMULATOR))
    target_sources(${EXOPROJECT_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/anode_hardware_port_halo12_va41630.c
            ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/anode_pwm_halo12_va41630.c
            )
endif()
