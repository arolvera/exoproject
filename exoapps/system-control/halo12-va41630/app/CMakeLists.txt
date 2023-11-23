message(STATUS " Adding Thruster Control Component")
target_sources(${EXOPROJECT_TARGET} PUBLIC
               ${CMAKE_CURRENT_SOURCE_DIR}/app.c
               )

if((NOT BUILD_SIMULATOR) OR (NOT ${BUILD_SIMULATOR}))
    target_sources(${EXOPROJECT_TARGET} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/main.c
            )
endif()

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)
