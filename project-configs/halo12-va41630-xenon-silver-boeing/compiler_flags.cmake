enable_language(ASM)

if(${BUILD_SIMULATOR})
    set(WARNING_FLAGS
            -Wno-missing-braces
            -Wno-unused-function
            )
    add_compile_options(${WARNING_FLAGS})
else()
#Set up compiler flags
set(CMAKE_ASM_FLAGS "-x assembler-with-cpp")
set(CPU_FLAGS
        -mcpu=cortex-m4
        -mfloat-abi=hard
        -mthumb
        #Use smaller library for float sprintf and printf
        --specs=nano.specs
        -Wl,--undefined,_scanf_float
        -Wl,--undefined,_printf_float
        )

set(CMAKE_C_FLAGS ${CPU_FLAGS} CACHE STRING "C Flags VA416XX")
set(WARNING_FLAGS
        -Wall
        -Wextra
#        -Wundef
        -Wno-unused-parameter
        -Wshadow=local
#        -Werror=stack-usage=102048
        -Wstack-usage=102048
        -Werror-implicit-function-declaration
        )

add_definitions(-D__VORAGO__)

add_compile_options(
        ${CPU_FLAGS}
#        -flto                          #Linker time optimization
        -fomit-frame-pointer            #
        -fno-common
        # Reduced code size
        -fno-unwind-tables
        -fno-exceptions
        # Remove unused code
        -fdata-sections
        -ffunction-sections
        -fmessage-length=0
        -fstack-usage
        -std=gnu11
        ${WARNING_FLAGS}
)

set(EXOPROJECT_LINKER_PATH "${CMAKE_CURRENT_LIST_DIR}" )
if(${BUILD_FREERTOS})
    set(EXOPROJECT_LINKER_SCRIPT "${EXOPROJECT_LINKER_PATH}/va416xx_linker_freertos.ld")
else()
    set(EXOPROJECT_LINKER_SCRIPT "${EXOPROJECT_LINKER_PATH}/va416xx_linker_baremetal.ld")
endif()

add_link_options(
        SHELL:${CPU_FLAGS}
        -Wlogical-op
        -nostartfiles
        --specs=nosys.specs              #No semi hosting function stubs
        -L${EXOPROJECT_LINKER_PATH}
        "SHELL:-T ${EXOPROJECT_LINKER_SCRIPT}"
        "LINKER:--gc-sections"
        "LINKER:-Map=${CMAKE_BINARY_DIR}/bin/${EXOPROJECT_TARGET}.map"
        "LINKER:--cref"
        "SHELL:LINKER:--start-group -lc -lm -Wl,--end-group"
        -Wl,--print-memory-usage
)
endif()
