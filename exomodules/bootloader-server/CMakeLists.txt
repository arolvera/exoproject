target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/bootloader_server.c
               )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )