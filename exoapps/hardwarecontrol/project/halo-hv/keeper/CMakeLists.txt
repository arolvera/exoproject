target_include_directories(${EXOPROJECT_TARGET} PRIVATE
                           .
                           )

target_sources(${EXOPROJECT_TARGET} PUBLIC
               ${CMAKE_CURRENT_SOURCE_DIR}/control/keeper_command_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/keeper_control_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/keeper_event_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/keeper_state_handlers_64M1_hv.c

               ${CMAKE_CURRENT_SOURCE_DIR}/initialize/keeper_initialize_64M1_hv.c
               )
