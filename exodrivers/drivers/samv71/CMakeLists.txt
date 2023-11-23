target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           ${CMAKE_CURRENT_SOURCE_DIR}/mcan
                           ${CMAKE_CURRENT_SOURCE_DIR}/mcan/include
                           ${CMAKE_CURRENT_SOURCE_DIR}/serial/
                           ${CMAKE_CURRENT_SOURCE_DIR}/serial/include)

target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/mcan/mcan_task.c
               ${CMAKE_CURRENT_SOURCE_DIR}/mcan/driver_mcan.c
               ${CMAKE_CURRENT_SOURCE_DIR}/serial/serial_task.c
        )