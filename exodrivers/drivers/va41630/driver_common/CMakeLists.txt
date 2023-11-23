target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)
target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/can/driver_can_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/ebi/driver_ebi_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/timer/driver_timer_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/adc/driver_adc_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/dac/driver_dac_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/hardware_port/driver_hardware_port_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/watchdog/driver_watchdog_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/serial/uart_driver_vorago.c
               ${CMAKE_CURRENT_SOURCE_DIR}/gpio/driver_gpio_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/fram/fram_va41630_cmn.c
               ${CMAKE_CURRENT_SOURCE_DIR}/flash/driver_nor_flash_va41630_cmn.c
               )
