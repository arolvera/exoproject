if(TRACE_UART)
    target_compile_definitions(${EXOPROJECT_TARGET} PUBLIC -DTRACE_UART)
endif()

target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/trace.c)
