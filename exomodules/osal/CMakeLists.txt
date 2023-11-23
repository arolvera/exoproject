if (${BUILD_FREERTOS})
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/osal_freertos.c
                   )
elseif(${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/osal_linux.c
                   )
else ()
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/osal_bm.c
                   )
endif ()

