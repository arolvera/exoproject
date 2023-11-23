target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/control_thruster_start.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control_thruster_start_${EXOMOD_SEQUENCE_FILE}.c
               )