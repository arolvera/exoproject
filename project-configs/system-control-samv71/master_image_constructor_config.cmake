set(EXOPROJECT_APP_TYPE     "master_image_constructor" CACHE STRING "" FORCE)
set(EXOPROJECT_APP  "halo12"           CACHE STRING "" FORCE)
set(EXODRIVER_PROCESSOR "va41630"          CACHE STRING "" FORCE)


#set configs for exomodules
set(BUILD_EXOMOD ON CACHE BOOL "Enable exomodules")
set(_EXOMOD_SUBDIRS
        #    component
        #    conditioning
        #    error
        #    iacm
        osal
        trace
        storage
        update
        #    sys
        #    sequence
        #    setting
        #    client-control
        #    utils
        )

set_property(GLOBAL
        PROPERTY EXOMOD_SUBDIRS
        ${_EXOMOD_SUBDIRS}
        )
set(EXOMOD_GAS  "krypton" CACHE STRING "sequence choice" FORCE)
set(EXOMOD_COIL "silver" CACHE STRING "sequence  choice" FORCE)
set(EXOMOD_CUSTOMER "boeing" CACHE STRING "sequence choice" FORCE)

#Enable what external libs need to be built
set(BUILD_CANOPEN OFF CACHE BOOL "Enable canopen")
set(BUILD_LIBCRC ON  CACHE BOOL  "Enable libcrc")
set(BUILD_FREERTOS OFF CACHE BOOL "Enable freertos")
set(BUILD_MIC ON CACHE BOOL "Enables master image constructor build")

add_link_options(-pthread)