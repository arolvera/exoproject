if(${BUILD_MASTER_IMAGE_CONSTRUCTOR})
    target_compile_definitions(${EXOPROJECT_TARGET} PUBLIC -DBUILD_MASTER_IMAGE_CONSTRUCTOR)
    set(_src_files PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_interface.c
            ${CMAKE_CURRENT_SOURCE_DIR}/storage_helper.c)
else()
    set(_src_files PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_interface.c
            ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory.c
            ${CMAKE_CURRENT_SOURCE_DIR}/storage_helper.c)
    add_subdirectory(storage-driver/${EXODRIVER_PROCESSOR})

endif()

add_subdirectory(memory-component/${EXOPROJECT_APP})

target_sources(${EXOPROJECT_TARGET} PRIVATE ${_src_files})

target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/include/
                            ${CMAKE_CURRENT_SOURCE_DIR}/memory-component/${EXOPROJECT_APP}/storage
                            ${CMAKE_CURRENT_SOURCE_DIR}
                           )


