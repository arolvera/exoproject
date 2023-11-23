target_sources(${EXOPROJECT_TARGET} PRIVATE
               ${CMAKE_CURRENT_SOURCE_DIR}/adc/driver_adc_64m1_bm.c
               ${CMAKE_CURRENT_SOURCE_DIR}/comp/driver_comp_64m1_bm.c
               ${CMAKE_CURRENT_SOURCE_DIR}/dac/driver_dac_64m1_bm.c
               ${CMAKE_CURRENT_SOURCE_DIR}/timer/driver_timer_64m1_bm.c
               ${CMAKE_CURRENT_SOURCE_DIR}/can/driver_can_64m1_bm.c
               )
