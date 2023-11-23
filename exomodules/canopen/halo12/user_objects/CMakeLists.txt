target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/anode_object.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/calib_vals_ucv_object.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/client_control_ucv_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/control_condition_object.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/error_detail_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/firmware_versions_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/health_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/hk_hsi_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/keeper_object.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/limits_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/magnets_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/magnets_object_${EXOPROJECT_APP}.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/mem_corrupter_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/power_control_object.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/sequence_table_update_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/throttle_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/thruster_command_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/trace_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/update_object.c
#               ${CMAKE_CURRENT_SOURCE_DIR}/user_config_var_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/valves_object.c
               ${CMAKE_CURRENT_SOURCE_DIR}/erase_nor_flash_object.c
               )

target_include_directories(${EXOPROJECT_TARGET} PUBLIC  ./)