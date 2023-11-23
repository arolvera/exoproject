target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/.
                           )

target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/component_service.c
               ${CMAKE_CURRENT_SOURCE_DIR}/component_communication.c
               )

if(NOT SINGLE_COMPONENT_BUILD)
    target_sources(${EXOPROJECT_TARGET} PRIVATE
                   ${CMAKE_CURRENT_SOURCE_DIR}/component_callback.c
                   )
endif()
