target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/client_control.c
               ${CMAKE_CURRENT_SOURCE_DIR}/client_lockout.c
               ${CMAKE_CURRENT_SOURCE_DIR}/client_service.c
               ${CMAKE_CURRENT_SOURCE_DIR}/client_health.c
               ${CMAKE_CURRENT_SOURCE_DIR}/client_booted.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control_autostart.c
               )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           include/
                           )

add_subdirectory(power)