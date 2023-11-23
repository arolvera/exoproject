target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/error_handler.c
               )

if(${BUILD_SIMULATOR})
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/fault_handler_nops.c
                   )
else()
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/fault_handler.c
                   )
endif()


target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})