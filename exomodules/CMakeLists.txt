target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/. ${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Sequence file name
set(EXOMOD_SEQUENCE_FILE "${EXOPROJECT_APP}_${EXOMOD_GAS}_${EXOMOD_COIL}_${EXOMOD_CUSTOMER}")

#Get sub dirs to enable
get_property(SUBDIRS GLOBAL PROPERTY EXOMOD_SUBDIRS)
foreach(subdir ${SUBDIRS})
    message(STATUS "Adding sub dir ${subdir}")
    add_subdirectory(${subdir})
endforeach()
