target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/control_setpoint.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control_setpoint_${EXOMOD_SEQUENCE_FILE}.c
               )