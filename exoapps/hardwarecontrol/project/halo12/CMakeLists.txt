exo_create_config_file(${CMAKE_CURRENT_SOURCE_DIR}/anode/include/anode_mcu_${EXOPROJECT_APP}.h.in
        ${EXOHARDWARE_BIN_DIR}/include/anode_mcu.h)
exo_create_config_file(${CMAKE_CURRENT_SOURCE_DIR}/keeper/include/keeper_mcu_${EXOPROJECT_APP}.h.in
        ${EXOHARDWARE_BIN_DIR}/include/keeper_mcu.h)
exo_create_config_file(${CMAKE_CURRENT_SOURCE_DIR}/magnet/include/magnet_mcu_${EXOPROJECT_APP}.h.in
        ${EXOHARDWARE_BIN_DIR}/include/magnet_mcu.h)
exo_create_config_file(${CMAKE_CURRENT_SOURCE_DIR}/valve/include/valve_mcu_${EXOPROJECT_APP}.h.in
        ${EXOHARDWARE_BIN_DIR}/include/valve_mcu.h)

target_include_directories(${EXOPROJECT_TARGET} PRIVATE ${EXOHARDWARE_BIN_DIR}/include/)

add_subdirectory(anode)
add_subdirectory(valve)
add_subdirectory(keeper)
add_subdirectory(magnet)
add_subdirectory(operation)
if((BUILD_SIMULATOR) AND (${BUILD_SIMULATOR}))
    add_subdirectory(hardware_mocks)
endif()

