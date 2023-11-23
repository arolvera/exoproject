target_sources(${EXOPROJECT_TARGET} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/health.c)

#Copy diag project specific file to binary include dir
exo_create_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/diag/diag_${EXOPROJECT_APP}.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/diag.h
)
target_include_directories(${EXOPROJECT_TARGET} PUBLIC
                           ./
                           ${CMAKE_CURRENT_BINARY_DIR}/include/
                           )
