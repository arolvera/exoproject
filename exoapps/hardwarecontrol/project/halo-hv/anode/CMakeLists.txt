target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/.
                           )

target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_command_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_control_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_event_handlers_64M1_hv.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control/anode_state_handlers_64M1_hv.c

               ${CMAKE_CURRENT_SOURCE_DIR}/initialize/anode_initialize_64M1_hv.c
               )
