# Get all subdirectories under ${current_dir} and store them
# in ${result} variable
macro(subdirlist result current_dir)
    file(GLOB children ${current_dir}/*)
    set(dirlist "")

    foreach (child ${children})
        if (IS_DIRECTORY ${child})
            list(APPEND dirlist ${child})
        endif ()
    endforeach ()

    set(${result} ${dirlist})
endmacro()

macro(print_subdir  current_dir)
    file(GLOB children ${current_dir}/*)
    foreach (child ${children})
        if (IS_DIRECTORY ${child})
            message(STATUS ${child})
        endif ()
    endforeach ()
endmacro()

function(directory_exists current_dir subdir result)
    subdirlist(_subdirs ${current_dir})
    set(_msg "")
    if(NOT ${subdir} IN_LIST ${_subdirs})
        string(REPLACE ";" "\n" _local_str ${_subdirs})
        message(FATAL_ERROR
                "directory ${subdir} not found\n "
                "options:\n "
                "${_local_str}"
                )
        set(result FALSE)
    else()
        set(result TRUE)
    endif()

endfunction()

# Prepend ${CMAKE_CURRENT_SOURCE_DIR} to a ${directory} name
# and save it in PARENT_SCOPE ${variable}
macro(prepend_cur_dir variable directory)
    set(${variable} ${CMAKE_CURRENT_SOURCE_DIR}/${directory})
endmacro()

# Add custom command to print firmware size in Berkley format
function(firmware_size target source_dir)
    get_target_property(_suffix ${target} SUFFIX)
    add_custom_command(TARGET ${target} POST_BUILD
                       COMMAND ${CMAKE_SIZE_UTIL} -B
                       "${source_dir}/${target}${_suffix}"
                       )
endfunction()

# Add a command to generate firmare in a provided format
function(generate_object target source_dir suffix type)
    get_target_property(_suffix ${target} SUFFIX)
    add_custom_command(TARGET ${target} POST_BUILD
                       COMMAND ${CMAKE_OBJCOPY} -O ${type}
                       "${source_dir}/${target}${_suffix}" "${source_dir}/${target}${suffix}"
                       )
endfunction()

# Add custom linker script to the linker flags
function(linker_script_add path_to_script)
    string(APPEND CMAKE_EXE_LINKER_FLAGS " -T ${path_to_script}")
endfunction()

# Update a target LINK_DEPENDS property with a custom linker script.
# That allows to rebuild that target if the linker script gets changed
function(linker_script_target_dependency target path_to_script)
    get_target_property(_cur_link_deps ${target} LINK_DEPENDS)
    string(APPEND _cur_link_deps " ${path_to_script}")
    set_target_properties(${target} PROPERTIES LINK_DEPENDS ${_cur_link_deps})
endfunction()

#Sets output dir for target. Resturns path in _path_to_out_dir
function(target_setup_binary _target _out_dir_name _path_to_out_dir )
    # CHange outputdir to <cmake-build-dir>/bin
    set_target_properties(${_target}
                          PROPERTIES
                          ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_out_dir_name}/lib"
                          LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_out_dir_name}/lib"
                          RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${_out_dir_name}/bin"
#                          OUTPUT_NAME              "${_bin_name}"
                          SUFFIX ".elf"
                          )
    get_target_property(_path_local ${_target} RUNTIME_OUTPUT_DIRECTORY)
    set(${_path_to_out_dir} ${_path_local} PARENT_SCOPE)
endfunction()

function(split_on_colon _colon_sep_list _out_str)
    #    set(_local_str "")
    string(REPLACE ";" "\n" _local_str ${_colon_sep_list})
    message(NOTICE "LOCAL ${_local_str}")
    #    set(${_out_str} ${_local_str} PARENT_SCOPE)
endfunction()

# Copies *.in file to dest location
macro(exo_create_config_file _src _dst)
    configure_file(${_src}
                   ${_dst}
                   IMMEDIATE @ONLY
                   )
    set_property(TARGET ${EXOPROJECT_TARGET}
                 APPEND
                 PROPERTY
                 ADDITIONAL_CLEAN_FILES ${_dst}
                 )
endmacro()

