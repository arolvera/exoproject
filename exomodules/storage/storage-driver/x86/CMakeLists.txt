target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_file_ops_driver.c
               ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_driver_ebi_nor_x86.c
               ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_driver_ebi_sram_x86.c
               ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_driver_eefc_x86.c
               ${CMAKE_CURRENT_SOURCE_DIR}/storage_memory_driver_fram_x86.c
               )

target_sources(${EXOPROJECT_TARGET} PUBLIC
               ${CMAKE_CURRENT_SOURCE_DIR}/.
               )