
target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(anode)
add_subdirectory(valve)
add_subdirectory(keeper)
add_subdirectory(magnet)
add_subdirectory(operation)