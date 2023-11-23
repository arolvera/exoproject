add_custom_target(temp_libcrc
        DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/lib/libcrc.a
        )

add_custom_command(
        OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/lib/libcrc.a"
        COMMAND make all -j1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Building libcrc"
        #        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/lib/libcrc.a
)

add_library(libcrc)

target_sources(libcrc
        PRIVATE
        src/crc8.c
        src/crc16.c
        src/crc32.c
        src/crc64.c
        src/crcccitt.c
        src/crcdnp.c
        src/crckrmit.c
        src/crcsick.c
        src/nmea-chk.c
        )

target_include_directories(libcrc BEFORE PUBLIC include)

add_dependencies(libcrc temp_libcrc)
