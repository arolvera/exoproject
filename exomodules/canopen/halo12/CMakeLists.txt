target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/driver/co_can_VA41630.c
               ${CMAKE_CURRENT_SOURCE_DIR}/driver/co_nvm_VA41630.c
               ${CMAKE_CURRENT_SOURCE_DIR}/driver/co_timer_VA41630.c
               ${CMAKE_CURRENT_SOURCE_DIR}/app_emcy.c
               ${CMAKE_CURRENT_SOURCE_DIR}/co_callbacks.c
               ${CMAKE_CURRENT_SOURCE_DIR}/thruster_control_spec.c
               ${CMAKE_CURRENT_SOURCE_DIR}/user_object_config.c
               )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ./)

add_subdirectory(driver)
add_subdirectory(user_objects)
