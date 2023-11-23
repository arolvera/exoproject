if(SINGLE_COMPONENT_BUILD)
    message(STATUS "Setting SINGLE_COMPONENT_BUILD")
#    target_compile_definitions(${EXOPROJECT_TARGET} PUBLIC -DSINGLE_COMPONENT_BUILD)
endif()

#Used to copy .in files to include folder
set(EXOHARDWARE_BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(operations/)
add_subdirectory(project/${EXOPROJECT_APP})
add_subdirectory(comp-interface/)
add_subdirectory(component/)

target_sources(${EXOPROJECT_TARGET} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/hwc_main.c
        )
