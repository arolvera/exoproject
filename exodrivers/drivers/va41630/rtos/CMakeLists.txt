target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)
target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/can/driver_can_va41630_rtos.c
               ${CMAKE_CURRENT_SOURCE_DIR}/serial/driver_serial_va41630_rtos.c
               ${CMAKE_CURRENT_SOURCE_DIR}/serial/dma_driver_vorago.c
               ${CMAKE_CURRENT_SOURCE_DIR}/serial/irq_router_driver_vorago.c
               )
