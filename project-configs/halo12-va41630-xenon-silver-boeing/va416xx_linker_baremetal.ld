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

/* Specify the memory areas */
MEMORY
{
	FLASH (rx)     : ORIGIN = 0x00000000, LENGTH = 256K
	SRAM1 (rw)     : ORIGIN = 0x1FFF8000, LENGTH = 32K
	SRAM2 (rw)     : ORIGIN = 0x1FFF8000 + LENGTH(SRAM1), LENGTH = 32K
	EBI_SRAM (rw)  : ORIGIN = 0x60000000, LENGTH = 0x80000 /* 512K for application data */
    EBI_SRAM_UPDATE (rw)  : ORIGIN = (ORIGIN(EBI_SRAM) + LENGTH(EBI_SRAM)), LENGTH = 0x80000 /* 512k for sw update upload */
    EBI_NOR  (rw)  : ORIGIN = 0x60000000 + LENGTH(EBI_SRAM) + LENGTH(EBI_SRAM_UPDATE), LENGTH = 0xE00000
}
ENTRY(Reset_Handler)
REGION_ALIAS("RAM", SRAM1);

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
    . = ALIGN(4);
    __edata = .;

    .data : AT(__edata)
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
    } > RAM
    __data_load_end__ = __data_load_start__ + SIZEOF(.data);
    __data_size_flash__ = SIZEOF(.data);

    .bss :
    {
        __bss_load_start__ = LOADADDR(.bss);
        __bss_run_start__ = ADDR(.bss);

        . = ALIGN(4);
        *(.bss*)
        . = ALIGN(4);
    } > RAM
    __bss_run_end__ = __bss_run_start__ + SIZEOF(.bss);

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
    } >EBI_NOR

    .ebi_sram :
    {
        EBI_SRAM_START_ADDR = ORIGIN(EBI_SRAM);
        EBI_SRAM_END_ADDR   = (EBI_SRAM_START_ADDR + LENGTH(EBI_SRAM));
        *(.ebi_sram);
        *(.ebi_sram*);
    } > EBI_SRAM

    /* Remove information from the standard libraries */
    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 : { *(.ARM.attributes) }
}
