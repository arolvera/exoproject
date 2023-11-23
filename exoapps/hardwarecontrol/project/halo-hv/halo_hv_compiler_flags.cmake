
add_definitions(-D__ATmega64M1__)
set(CPU_FLAGS
    -mmcu=atmega64m1
    -Wl,--gc-sections
    -O1
    --sysroot="${CMAKE_SOURCE_DIR}/bsp/xc8"
    )

set(WARNING_FLAGS
    -Wall
    )

add_compile_options(
    ${CPU_FLAGS}
    -c
    -ffunction-sections
    -fdata-sections
    -fpack-struct
    -fshort-enums
    -funsigned-char
    -funsigned-bitfields
    -fno-common
    -gdwarf-3
)

add_link_options(
    "SHELL:${CPU_FLAGS}"
    "SHELL:-Wl,--start-group -Wl,-lm -Wl,--end-group"
)


