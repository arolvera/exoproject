add_library(cmsis INTERFACE)
target_include_directories(cmsis INTERFACE
                           cmsis/
                           )

exo_add_extra_lib(cmsis)