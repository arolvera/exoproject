if(${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    set(_src_list ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_component_fcp.c)
else()
    set(_src_list ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_component_fcp.c
                  ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_file_map.c)
endif()

target_sources(${EXOPROJECT_TARGET} PRIVATE
        ${_src_list}
        )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/storage
        )
