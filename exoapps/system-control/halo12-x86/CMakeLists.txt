if("${EXOPROJECT_APP_SUBTYPE}" STREQUAL "hardwarecontrol-emu")
    add_subdirectory(${EXOPROJECT_APP_SUBTYPE})
elseif("${EXOPROJECT_APP_SUBTYPE}" STREQUAL "system-control-emu")
    add_subdirectory(${EXOPROJECT_APP_SUBTYPE})
endif()
#
#if(NOT "${EXOPROJECT_APP_SUBTYPE}" STREQUAL "app" AND BUILD_SIMULATOR)
#    add_subdirectory(${EXOPROJECT_APP_SUBTYPE})
#endif()

# why are these libraries added at this level?
exo_add_extra_lib(rt)
exo_add_extra_lib(pthread)
exo_add_extra_lib(pcanbasic)
exo_add_extra_lib(m)
