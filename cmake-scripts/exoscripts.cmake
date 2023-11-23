function(exo_add_extra_lib _target)
    set_property(TARGET ${EXOPROJECT_TARGET} APPEND PROPERTY EXOLIBS ${_target})
endfunction()

function(exo_get_extra_libs _extra_libs)
    get_property(was_set TARGET ${EXOPROJECT_TARGET} PROPERTY EXOLIBS SET)
    if(was_set)
        get_target_property(_tmp  ${EXOPROJECT_TARGET}  EXOLIBS)
        set(${_extra_libs} ${_tmp} PARENT_SCOPE)
    endif()
endfunction()

macro(exo_print_list _list)
    foreach(_l ${_list})
        message(STATUS "${_l}")
    endforeach()
endmacro()

#Generates project name based off of
#TYPE_PROJECT_PROCESSOR_GAS_COIL_CUSTOMER
function(exo_generate_project_name)
    set(_exoname_list "${EXOPROJECT_APP_TYPE}-"
                        "${EXOPROJECT_APP}-"
                        "${EXODRIVER_PROCESSOR}-"
                        "${EXOMOD_GAS}-"
                        "${EXOMOD_COIL}-"
                        "${EXOMOD_CUSTOMER}"
                        )
    string(REPLACE ";" "_" _exoname ${_exoname_list})

    if(${BUILD_PROJECT_BOOTLOADER})
        set(_exoname bootloader-${_exoname})
    endif()

    set(EXOPROJECT_TARGET "${_exoname}" CACHE STRING "Name of project target")
endfunction()

