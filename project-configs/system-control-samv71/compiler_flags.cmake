enable_language(C ASM)

#Set up compiler flags
set(ASM_OPTIONS "-g -x c -c -x assembler-with-cpp")
set(CMAKE_ASM_FLAGS ${CFLAGS} )

set(CMAKE_C_FLAGS ${CPU_FLAGS} CACHE STRING "C Flags VA416XX")
set(WARNING_FLAGS
        -Wall
        )

add_definitions(-DXPRJ_default=default)


set(EXOPROJECT_LINKER_PATH "${CMAKE_CURRENT_LIST_DIR}")
set(EXOPROJECT_LINKER_SCRIPT "${EXOPROJECT_LINKER_PATH}/ATSAMV71Q21B.ld")
set(EXOPROJECT_LINKER_OPTS         "-L${EXOPROJECT_LINKER_PATH}"
                                   "SHELL:-T ${EXOPROJECT_LINKER_SCRIPT}"
                                   "LINKER:-Map=${CMAKE_BINARY_DIR}/${EXOPROJECT_TARGET}.map,--defsym=_min_heap_size=512,--gc-sections")

if(${EXOPROJECT_APP_TYPE} STREQUAL "system-control")
    # Elevate all warnings to errors for the application
    # The bootloader build for SAMV71 projects will error out on a warning about the #pragma config <SETTING> in
    #  initialization.c. This #pragma sets bits in the GPNVM memory, which allow the SAM to boot from flash rather than ROM.
    #  Several workarounds are available for this, but the most straight forward at this time is
    #  just setting the fuses manually using the mplabs IDE.  This can also be accomplished by adding the assembly
    #  instructions to the production .hex, but this is fairly high risk and not the most elegant solution.
    # NOTE: The correct fix is to find out why the build can't recognize the config word, but at the time of this
    #  writing I have been unable to dig up any solutions.
    set(WARNING_FLAGS
            -Wall
            -Werror
            )
    set(EXOPROJECT_LINKER_OPTS "${EXOPROJECT_LINKER_OPTS}" "LINKER:-DROM_LENGTH=0x1f0000,-DROM_ORIGIN=0x410000")
endif()

add_compile_options(
        ${CPU_FLAGS}
        -mdfp=${CMAKE_CURRENT_SOURCE_DIR}/project-configs/system-control-samv71
        -ffunction-sections
        -fdata-sections
        -O1
        -DBUILD_CONFIG=1
        ${WARNING_FLAGS}
)

#This is from the auto_generated makefiles. App does not need start-up code since it has a bootloader
add_link_options(${EXOPROJECT_LINKER_OPTS})

