if(${BUILD_FREERTOS})
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/msg_handler_rtos.c
                   )
else()
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/msg_handler_bm.c
                   )
endif()

