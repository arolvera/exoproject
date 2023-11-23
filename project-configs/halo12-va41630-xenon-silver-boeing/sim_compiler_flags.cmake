#enable_language(ASM)

set(WARNING_FLAGS
        -Wno-missing-braces
        -Wno-unused-function
        -ggdb
        -rdynamic
        )
add_compile_options(${WARNING_FLAGS})
add_link_options(
    -no-pie
    -rdynamic
)