target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/task_monitor.c
               ${CMAKE_CURRENT_SOURCE_DIR}/component_tasks.c
               )

add_subdirectory(${EXOPROJECT_APP})