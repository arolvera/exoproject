if(${BUILD_FREERTOS})
    add_subdirectory(rtos/)
else()
    add_subdirectory(bm/)
endif()
add_subdirectory(driver_common/)
