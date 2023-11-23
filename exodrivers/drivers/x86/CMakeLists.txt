target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)
target_sources(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/timer/driver_timer_x86_cmn.c
        ${CMAKE_CURRENT_SOURCE_DIR}/hardware_port/driver_hardware_port_va41630_cmn.c
        ${CMAKE_CURRENT_SOURCE_DIR}/serial/driver_serial_va41630_rtos.c
        ${CMAKE_CURRENT_SOURCE_DIR}/serial/uart_driver_vorago.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ebi/driver_ebi_va41630_cmn.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gpio/driver_gpio_x86_cmn.c
)

if((SIMULATOR_USE_SOCKET) AND (${SIMULATOR_USE_SOCKET}))
    target_sources(${EXOPROJECT_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/can/driver_socket_can_x86.c
            )
else()
    target_sources(${EXOPROJECT_TARGET} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/can/driver_can_x86.c
            )
endif()
