target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/control_throttle.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control_throttle_${EXOMOD_SEQUENCE_FILE}.c
               )