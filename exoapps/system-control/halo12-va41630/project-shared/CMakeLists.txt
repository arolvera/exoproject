target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/include/
                           ${CMAKE_CURRENT_SOURCE_DIR}/.
                           ${CMAKE_CURRENT_SOURCE_DIR}/freertos/
                           )

if(${BUILD_FREERTOS})
    add_subdirectory(freertos/)
endif()

add_subdirectory(mcu/${EXODRIVER_PROCESSOR})
