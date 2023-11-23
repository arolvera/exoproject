set(EXODRIVER_PROCESSOR "" CACHE STRING "Choose chip drivers")
if(NOT EXODRIVER_PROCESSOR)
    message(FATAL_ERROR
            "Set EXODRIVER_PROCESSOR \n"
            "options: \n"
            "atmega64m1"
            "va41630"
            )
endif()

add_subdirectory(hal/)

add_subdirectory(drivers/${EXODRIVER_PROCESSOR})
