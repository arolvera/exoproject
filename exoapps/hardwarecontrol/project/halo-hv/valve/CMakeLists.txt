target_include_directories(${EXOPROJECT_TARGET} PRIVATE
                           .
                           )

target_sources(${EXOPROJECT_TARGET} PUBLIC
               ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_command_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_control_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_event_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/valve_state_handlers_64M1_hv.c

               ${CMAKE_CURRENT_SOURCE_DIR}/initialize/valve_initialize_64M1_hv.c
               )
