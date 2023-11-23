add_library(percepio STATIC
            ${CMAKE_CURRENT_SOURCE_DIR}/trcTask.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcAssert.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcCounter.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcDiagnostics.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcEntryTable.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcError.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcEvent.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcEventBuffer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcExtension.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcHardwarePort.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcHeap.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcInternalEventBuffer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcInterval.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcISR.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcMultiCoreEventBuffer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcObject.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcPrint.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcSnapshotRecorder.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcStackMonitor.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcStateMachine.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcStaticBuffer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcStreamingRecorder.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcString.c
            ${CMAKE_CURRENT_SOURCE_DIR}/trcTimestamp.c
            ${CMAKE_CURRENT_SOURCE_DIR}/streamports/Jlink_RTT/trcStreamPort.c
            ${CMAKE_CURRENT_SOURCE_DIR}/streamports/Jlink_RTT/SEGGER_RTT.c
            ${CMAKE_CURRENT_SOURCE_DIR}/kernelports/FreeRTOS/trcKernelPort.c
            ${CMAKE_CURRENT_SOURCE_DIR}/RTT/SEGGER_RTT.c
            ${CMAKE_CURRENT_SOURCE_DIR}/RTT/SEGGER_RTT_printf.c
            ${CMAKE_CURRENT_SOURCE_DIR}/RTT/SEGGER_SYSVIEW.c
            ${CMAKE_CURRENT_SOURCE_DIR}/RTT/SEGGER_RTT_ASM_ARMv7M.S
            )

target_include_directories(percepio PUBLIC
                           ${CMAKE_CURRENT_SOURCE_DIR}/include
                           ${CMAKE_CURRENT_SOURCE_DIR}/percepio_config
                           ${CMAKE_CURRENT_SOURCE_DIR}/kernelports/FreeRTOS/config
                           ${CMAKE_CURRENT_SOURCE_DIR}/kernelports/FreeRTOS/include
                           ${CMAKE_CURRENT_SOURCE_DIR}/kernelports/FreeRTOS/streamports
                           ${CMAKE_CURRENT_SOURCE_DIR}/streamports/Jlink_RTT/include
                           ${CMAKE_CURRENT_SOURCE_DIR}/streamports/Jlink_RTT/config
                           ${CMAKE_CURRENT_SOURCE_DIR}/segger_config
                           ${CMAKE_CURRENT_SOURCE_DIR}/RTT/
                           )


target_link_libraries(percepio freertos_kernel cmsis)