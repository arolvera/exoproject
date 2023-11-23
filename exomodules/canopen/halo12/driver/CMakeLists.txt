target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/co_can_VA41630.c
               ${CMAKE_CURRENT_SOURCE_DIR}/co_nvm_VA41630.c
               ${CMAKE_CURRENT_SOURCE_DIR}/co_timer_VA41630.c
               )

target_include_directories(${EXOPROJECT_TARGET} PRIVATE
                           ./
                           )