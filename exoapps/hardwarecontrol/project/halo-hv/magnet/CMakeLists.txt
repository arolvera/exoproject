target_include_directories(${EXOPROJECT_TARGET} PRIVATE
                           .
                           )

target_sources(${EXOPROJECT_TARGET} PUBLIC
               ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_command_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_control_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_event_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/magnet_state_handlers_64M1_hv.c

               ${CMAKE_CURRENT_SOURCE_DIR}/initialize/magnet_initialize_64M1_hv.c
               )
