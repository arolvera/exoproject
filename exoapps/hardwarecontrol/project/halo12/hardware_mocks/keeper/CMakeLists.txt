target_include_directories(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
        )

target_sources(${EXOPROJECT_TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/keeper_hardware_port_halo12_va41630.c
        ${CMAKE_CURRENT_SOURCE_DIR}/component-drivers/va41630/keeper_pwm_halo12_va41630.c
        )
