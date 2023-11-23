if(${BUILD_CMSIS})
    message(STATUS "Building CMSIS")
    add_subdirectory(cmsis)
endif()

#Add submodule libs
if (${BUILD_LIBCRC})
    message(STATUS "Building libcrc")
    add_subdirectory(libcrc-utility)
    exo_add_extra_lib(libcrc)
endif ()

#add Freertos
if (${BUILD_FREERTOS})
    message(STATUS "Building freertos_kernel")
    add_subdirectory(freertos)
    exo_add_extra_lib(freertos_kernel)
endif()

#add percepio
if (${BUILD_PERCEPIO})
    message(STATUS "Building Percepio")
    add_subdirectory(percepio)
    exo_add_extra_lib(percepio)
endif()

#add canopen-stack
if (${BUILD_CANOPEN})
    message(STATUS "Building canopen")
    add_subdirectory(canopen-stack)
    exo_add_extra_lib(Canopen)
endif ()

