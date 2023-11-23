target_sources(${EXOPROJECT_TARGET} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/sequence_test.c
        ${CMAKE_CURRENT_SOURCE_DIR}/app.c
        )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)