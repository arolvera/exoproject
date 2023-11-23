target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/control_condition.c
               )

exo_create_config_file(${CMAKE_CURRENT_SOURCE_DIR}/private-include/seq_cond_${EXOMOD_SEQUENCE_FILE}.h.in
                       ${CMAKE_CURRENT_BINARY_DIR}/include/seq_cond_config.h)

target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ${CMAKE_CURRENT_BINARY_DIR}/include/
                           ${CMAKE_CURRENT_SOURCE_DIR}/private-include/
                           )
