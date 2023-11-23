if (${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_component_halo12.c)
elseif((NOT BUILD_BOOTLOADER) OR (NOT ${BUILD_BOOTLOADER}))
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_component_halo12.c
                   ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_file_map.c
                   )
endif ()

target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}
                           )
