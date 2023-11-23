target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/control_anode.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control_anode_${EXOPROJECT_APP}.c
               )
target_include_directories(${EXOPROJECT_TARGET} PUBLIC ./)