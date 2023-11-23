/******************************************************************************
 * This linker file was developed from the example file included in the GCC download.
 *  <..>\GCC_install\gcc-arm-none-eabi-7-2017-q4-major-win32\share\gcc-arm-none-eabi\samples\ldscripts
 *
 * The author is not responsible for any
 * damage or liability that this file might cause.
******************************************************************************/

 /*   Modified for VA416xx MCU:
            Cortex M4 based
            256k byte of Instruction space RAM at 0x0000:0000
            32k byte of Data space RAM at 0x1FFF:8000
*/

/* Entry Point */

/* Specify the memory areas. There is a difference between the dev board with a memory hat and the
   ECPK board with on-board memory. The ECPK allows 512k for both app and update regions but the
   dev board limits to 256k. To test on the dev board, uncomment the first MEMORY definition.
   To test on a PPU board, uncomment the second one. */
MEMORY
{
	FLASH (rx)     : ORIGIN = 0x00010000, LENGTH = 196K
	SRAM1 (rw)     : ORIGIN = 0x1FFF8000, LENGTH = 32K
	SRAM2 (rw)     : ORIGIN = 0x1FFF8000 + LENGTH(SRAM1), LENGTH = 30K
	RTT (rw)       : ORIGIN = 0x1FFF8000 + LENGTH(SRAM1) + LENGTH(SRAM2), LENGTH = 2K
    EBI_SRAM (rw)  : ORIGIN = 0x60000000, LENGTH = 0x13000 /* 256K for application data */
    EBI_SRAM_UPDATE (rw)  : ORIGIN = (ORIGIN(EBI_SRAM) + LENGTH(EBI_SRAM)), LENGTH = 0x2d000 /* 256k for sw update upload */
    EBI_NOR  (rw)  : ORIGIN = 0x60000000 + LENGTH(EBI_SRAM) + LENGTH(EBI_SRAM_UPDATE), LENGTH = 0xE00000
}

ENTRY(Reset_Handler)

SECTIONS
{
    .text : ALIGN(4)
    {
        KEEP(*(.isr_vector))
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        *(.glue_7)         /* glue arm to thumb code */
        *(.glue_7t)        /* glue thumb to arm code */

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > FLASH

    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx : ALIGN(4)
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    .freertos_stack1 : ALIGN(4)
    {
        __freertos_1_load_start__ = LOADADDR(.freertos_stack1);
        __freertos_1_run_start__ = ADDR(.freertos_stack1);
        *(*.tc_node)
        *(*.CanOpenDriver)
        *(*.ATSAMV71TimerDriver)
        *(*.ATSAMV71NvmDriver)
    } >SRAM1 AT> FLASH
    __freertos_1_load_end__ = __freertos_1_load_start__ + SIZEOF(.freertos_stack1);
    . = __freertos_1_load_end__;

    .data : ALIGN(4)
    {
        __data_load_start__ = LOADADDR(.data);
        __data_run_start__ = ADDR(.data);

        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);

        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;

        . = ALIGN(4);
    } > EBI_SRAM AT>FLASH
    __data_load_end__ = __data_load_start__ + SIZEOF(.data);
    . = __data_load_end__;

    .freertos_stack2 : ALIGN(4)
    {
        __freertos_2_load_start__ = LOADADDR(.freertos_stack2);
        __freertos_2_run_start__ = ADDR(.freertos_stack2);
        *(*.tc_task_stack)
        *(*.health_timer_stack)
        *(*.keeper_task_stack)
        *(*.magnet_task_stack)
        *(*.anode_task_stack)
        *(*.autostart_task_stack)
        *(*.client_task_stack)
        *(*.coClockStack)
        *(*.msg_handler_task_stack)
        *(*.cond_task_stack)
        *(*.seq_task_stack)
    } > SRAM2 AT> FLASH
    __freertos_2_run_end__ = __freertos_2_run_start__ + SIZEOF(.freertos_stack2);
    __freertos_2_load_end__ = __freertos_2_load_start__ + SIZEOF(.freertos_stack2);
    . = __freertos_2_load_end__;

    .bss : ALIGN(4)
    {
        __bss_load_start__ = LOADADDR(.bss);
        __bss_run_start__ = ADDR(.bss);

        . = ALIGN(4);
        *(.bss*)
        . = ALIGN(4);
    } > EBI_SRAM AT> FLASH
    __bss_run_end__ = __bss_run_start__ + SIZEOF(.bss);


    .jlink_rtt (NOLOAD) :
    {
        . = ALIGN(4);
        RTT_START_ADDR = ORIGIN(RTT);
        RTT_END_ADDR   = (RTT_START_ADDR + LENGTH(RTT) - 4);
    } >RTT


    .heap (NOLOAD):
    {
        __end__ = .;
        end = __end__;
        *(.heap*)
        __HeapLimit = .;
    } > SRAM1

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack (NOLOAD):
    {
        *(.stack*)
    } > SRAM1

    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(SRAM1) + LENGTH(SRAM1);
    __StackLimit = __StackTop - SIZEOF(.stack);
    PROVIDE(__stack = __StackTop);

   .ebi_nor (NOLOAD) :
    {
        EBI_NOR_START_ADDR = ORIGIN(EBI_NOR);
        EBI_NOR_END_ADDR   = (EBI_NOR_START_ADDR + LENGTH(EBI_NOR));
        *(.ebi_nor);
        *(.ebi_nor*);
    } > EBI_NOR

    .ebi_sram :
    {
        EBI_SRAM_START_ADDR = ORIGIN(EBI_SRAM);
        /*EBI_SRAM_END_ADDR is used to set up EBI perph. Add missing 500K so we can put update there*/
        EBI_SRAM_END_ADDR   = (EBI_SRAM_START_ADDR + LENGTH(EBI_SRAM_UPDATE) + LENGTH(EBI_SRAM));
        *(.ebi_sram);
        *(.ebi_sram*);
    } > EBI_SRAM

    .ebi_sram_update (NOLOAD) :
    {
        *(.sys_ctrl_update_space);
        *(.sys_ctrl_update_space*);
    } > EBI_SRAM_UPDATE

    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
