
target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/app_dict.c
               ${CMAKE_CURRENT_SOURCE_DIR}/co_task.c
               )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

add_subdirectory(${EXOPROJECT_APP})