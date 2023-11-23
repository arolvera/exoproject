target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/binary_range_tree.c
               ${CMAKE_CURRENT_SOURCE_DIR}/circular_buffer.c
               ${CMAKE_CURRENT_SOURCE_DIR}/stack.c
               )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})