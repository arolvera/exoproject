if("${BUILD_MASTER_IMAGE_CONSTRUCTOR}" OR "${BUILD_PROJECT_BOOTLOADER}")
    set(_src_files PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/update_helper.c)
else()
    set(_src_files PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/update_helper.c
                           ${CMAKE_CURRENT_SOURCE_DIR}/update_command.c)
endif()

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${_src_files}
               )