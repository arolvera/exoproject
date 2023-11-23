target_include_directories(${EXOPROJECT_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/.)
target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/can/driver_can_va41630_bm.c
               )