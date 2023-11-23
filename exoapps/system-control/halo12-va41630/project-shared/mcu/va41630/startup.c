#include <stdint.h>
#include "device.h"
#include "sys_init.h"
#include "flash/hal_flash.h"
#ifdef FREE_RTOS
extern uint32_t __freertos_1_load_start__;
extern uint32_t __freertos_1_load_end__;
extern uint32_t __freertos_1_run_start__;

extern uint32_t __freertos_2_load_start__;
extern uint32_t __freertos_2_run_start__;
extern uint32_t __freertos_2_run_end__;
#endif
extern uint32_t __StackTop;
extern uint32_t __data_load_start__;
extern uint32_t __data_load_end__;
extern uint32_t __data_run_start__;

extern uint32_t __bss_run_start__;
extern uint32_t __bss_run_end__;

extern int main(void);         // Application main entry point

//vector table defined in startup.c
extern const void *vector_table[];

//The internet does not recommend using memcpy in startup. Not gaunted everything is set up.
void copy_mem(uint32_t *srcaddr_start, const uint32_t *srcaddr_end, uint32_t *dstaddr_start, uint32_t *dstaddr_end)
{
    uint32_t *src = srcaddr_start;
    uint32_t *dst = dstaddr_start;

    // Copying data section from flash to RAM.
    if(srcaddr_start) {
        while(src < srcaddr_end) {
            *dst++ = *src++;
        }
    }else {
        while(dst < dstaddr_end) {
            *dst++ = 0;
        }
    }
}

void Reset_Handler(void)
{


    //Init vector tables
    SCB->VTOR = (uint32_t)vector_table;

    SCB->CPACR |= ((3U << 10U * 2U) | /* set CP10 Full Access */
        (3U << 11U * 2U)); /* set CP11 Full Access */

    sys_ebi_init();

    flash_init();

#ifdef FREE_RTOS
    copy_mem((uint32_t *)&__freertos_1_load_start__, (uint32_t*)&__freertos_1_load_end__, (uint32_t *) &__freertos_1_run_start__, 0);
    copy_mem((uint32_t *)0, (uint32_t*)0, (uint32_t *)&__freertos_2_run_start__, &__freertos_2_run_end__);
#endif
    copy_mem((uint32_t *)&__data_load_start__, (uint32_t*)&__data_load_end__, (uint32_t *)&__data_run_start__, 0);

    // Clearing the bss section.
    copy_mem((uint32_t *)0, (uint32_t*)0, (uint32_t *)&__bss_run_start__, &__bss_run_end__);

    // Calling application main entry point.
    main();
    // We should never return from main.
    for(;;);
}


void default_handler(void)    // Default interrupt handler
{
#ifndef __DEBUG
    asm("TST LR, #4\n\t"
        "ITE EQ\n\t"
        "MRSEQ R0, MSP\n\t"
        "MRSNE R0, PSP\n\t"
        "B hard_fault_print");
#endif
    sys_stat_crash_set(SYS_CRASH_TRUE);
    for(;;);
}

void hard_fault_print(uint32_t *hardfault_args)
{
    volatile uint32_t stacked_r0;
    volatile uint32_t stacked_r1;
    volatile uint32_t stacked_r2;
    volatile uint32_t stacked_r3;
    volatile uint32_t stacked_r12;
    volatile uint32_t stacked_lr;
    volatile uint32_t stacked_pc;
    volatile uint32_t stacked_psr;

    stacked_r0 = ((unsigned long)hardfault_args[0]);
    stacked_r1 = ((unsigned long)hardfault_args[1]);
    stacked_r2 = ((unsigned long)hardfault_args[2]);
    stacked_r3 = ((unsigned long)hardfault_args[3]);

    stacked_r12 = ((unsigned long)hardfault_args[4]);
    stacked_lr = ((unsigned long)hardfault_args[5]);
    stacked_pc = ((unsigned long)hardfault_args[6]);
    stacked_psr = ((unsigned long)hardfault_args[7]);

    (void)stacked_r0;
    (void)stacked_r1;
    (void)stacked_r2;
    (void)stacked_r3;
    (void)stacked_r12;
    (void)stacked_lr;
    (void)stacked_pc;
    (void)stacked_psr;
    sys_stat_crash_set(SYS_CRASH_TRUE);
    __BKPT(0);

}

void NMI_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void HardFault_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void MemManage_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void BusFault_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void SVC_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void DebugMon_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void PendSV_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void SysTick_Handler(void) __attribute__ ((weak, alias ("default_handler")));
void OC0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI0_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI0_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI1_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI1_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI2_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI2_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI3_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SPI3_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void UART0_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void UART0_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void UART1_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void UART1_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void UART2_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void UART2_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C0_MS_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C0_SL_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C1_MS_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C1_SL_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C2_MS_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C2_SL_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void Ethernet_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC37_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void SpW_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC39_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DAC0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DAC1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TRNG_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Error_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void ADC_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void LoCLK_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void LVD_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void WDT_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM16_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM17_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM18_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM19_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM20_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM21_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM22_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TIM23_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void CAN0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC73_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void CAN1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void OC75_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void EDAC_MBE_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void EDAC_SBE_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PA15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PB15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PC15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PD15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PE15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF4_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF5_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF6_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF7_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF8_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF9_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF10_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF11_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF12_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF13_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF14_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void PF15_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Active_0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Active_1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Active_2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Active_3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Done_0_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Done_1_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Done_2_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void DMA_Done_3_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C0_MS_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C0_MS_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C0_SL_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C0_SL_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C1_MS_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C1_MS_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C1_SL_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C1_SL_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C2_MS_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C2_MS_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C2_SL_RX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void I2C2_SL_TX_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void FPU_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));
void TXEV_IRQHandler(void) __attribute__ ((weak, alias ("default_handler")));

// Interrupts vectors table
const void *vector_table[] __attribute__ ((section(".isr_vector"))) = {
    &__StackTop,            /* Top of Stack */
    Reset_Handler,         /* Reset Handler */
    NMI_Handler,          /* NMI Handler */
    HardFault_Handler,     /* Hard Fault Handler */
    MemManage_Handler,     /* MPU Fault Handler */
    BusFault_Handler,      /* Bus Fault Handler */
    UsageFault_Handler,    /* Usage Fault Handler */
    0,                     /* Reserved */
    0,                     /* Reserved */
    0,                     /* Reserved */
    0,                     /* Reserved */
    SVC_Handler,           /* SVCall Handler */
    DebugMon_Handler,      /* Debug Monitor Handler */
    0,                     /* Reserved */
    PendSV_Handler,        /* PendSV Handler */
    SysTick_Handler,       /* SysTick Handler */

// External interrupts
    OC0_IRQHandler, //   0:  Always 0
    OC1_IRQHandler, //   1:  Always 0
    OC2_IRQHandler,//   2:  Always 0
    OC3_IRQHandler,//   3:  Always 0
    OC4_IRQHandler,//   4:  Always 0
    OC5_IRQHandler,//   5:  Always 0
    OC6_IRQHandler,//   6:  Always 0
    OC7_IRQHandler,//   7:  Always 0
    OC8_IRQHandler,//   8:  Always 0
    OC9_IRQHandler,//   9:  Always 0
    OC10_IRQHandler,//  10:  Always 0
    OC11_IRQHandler,//  11:  Always 0
    OC12_IRQHandler,//  12:  Always 0
    OC13_IRQHandler,//  13:  Always 0
    OC14_IRQHandler,//  14:  Always 0
    OC15_IRQHandler,//  15:  Always 0
    SPI0_TX_IRQHandler,//  16
    SPI0_RX_IRQHandler,//  17
    SPI1_TX_IRQHandler,//  18
    SPI1_RX_IRQHandler,//  19
    SPI2_TX_IRQHandler,//  20
    SPI2_RX_IRQHandler,//  21
    SPI3_TX_IRQHandler,//  22
    SPI3_RX_IRQHandler,//  23
    UART0_TX_IRQHandler,//  24
    UART0_RX_IRQHandler,//  25
    UART1_TX_IRQHandler,//  26
    UART1_RX_IRQHandler,//  27
    UART2_TX_IRQHandler,//  28
    UART2_RX_IRQHandler,//  29
    I2C0_MS_IRQHandler,//  30
    I2C0_SL_IRQHandler,//  31
    I2C1_MS_IRQHandler,//  32
    I2C1_SL_IRQHandler,//  33
    I2C2_MS_IRQHandler,//  34
    I2C2_SL_IRQHandler,//  35
    Ethernet_IRQHandler,//  36
    OC37_IRQHandler,//  37
    SpW_IRQHandler,//  38
    OC39_IRQHandler,//  39
    DAC0_IRQHandler,//  40
    DAC1_IRQHandler,//  41
    TRNG_IRQHandler,//  42
    DMA_Error_IRQHandler,//  43
    ADC_IRQHandler,//  44
    LoCLK_IRQHandler,//  45
    LVD_IRQHandler,//  46
    WDT_IRQHandler,//  47
    TIM0_IRQHandler,//  48
    TIM1_IRQHandler,//  49
    TIM2_IRQHandler,//  50
    TIM3_IRQHandler,//  51
    TIM4_IRQHandler,//  52
    TIM5_IRQHandler,//  53
    TIM6_IRQHandler,//  54
    TIM7_IRQHandler,//  55
    TIM8_IRQHandler,//  56
    TIM9_IRQHandler,//  57
    TIM10_IRQHandler,//  58
    TIM11_IRQHandler,//  59
    TIM12_IRQHandler,//  60
    TIM13_IRQHandler,//  61
    TIM14_IRQHandler,//  62
    TIM15_IRQHandler,//  63
    TIM16_IRQHandler,//  64
    TIM17_IRQHandler,//  65
    TIM18_IRQHandler,//  66
    TIM19_IRQHandler,//  67
    TIM20_IRQHandler,//  68
    TIM21_IRQHandler,//  69
    TIM22_IRQHandler,//  70
    TIM23_IRQHandler,//  71
    CAN0_IRQHandler,//  72
    OC73_IRQHandler,//  73
    CAN1_IRQHandler,//  74
    OC75_IRQHandler,//  75
    EDAC_MBE_IRQHandler,//  76
    EDAC_SBE_IRQHandler,//  77
    PA0_IRQHandler,//  78
    PA1_IRQHandler,//  79
    PA2_IRQHandler,//  80
    PA3_IRQHandler,//  81
    PA4_IRQHandler,//  82
    PA5_IRQHandler,//  83
    PA6_IRQHandler,//  84
    PA7_IRQHandler,//  85
    PA8_IRQHandler,//  86
    PA9_IRQHandler,//  87
    PA10_IRQHandler,//  88
    PA11_IRQHandler,//  89
    PA12_IRQHandler,//  90
    PA13_IRQHandler,//  91
    PA14_IRQHandler,//  92
    PA15_IRQHandler,//  93
    PB0_IRQHandler,//  94
    PB1_IRQHandler,//  95
    PB2_IRQHandler,//  96
    PB3_IRQHandler,//  97
    PB4_IRQHandler,//  98
    PB5_IRQHandler,//  99
    PB6_IRQHandler,//  100
    PB7_IRQHandler,//  101
    PB8_IRQHandler,//  102
    PB9_IRQHandler,//  103
    PB10_IRQHandler,//  104
    PB11_IRQHandler,//  105
    PB12_IRQHandler,//  106
    PB13_IRQHandler,//  107
    PB14_IRQHandler,//  108
    PB15_IRQHandler,//  109
    PC0_IRQHandler,//  110
    PC1_IRQHandler,//  111
    PC2_IRQHandler,//  112
    PC3_IRQHandler,//  113
    PC4_IRQHandler,//  114
    PC5_IRQHandler,//  115
    PC6_IRQHandler,//  116
    PC7_IRQHandler,//  117
    PC8_IRQHandler,//  118
    PC9_IRQHandler,//  119
    PC10_IRQHandler,//  120
    PC11_IRQHandler,//  121
    PC12_IRQHandler,//  122
    PC13_IRQHandler,//  123
    PC14_IRQHandler,//  124
    PC15_IRQHandler,//  125
    PD0_IRQHandler,//  126
    PD1_IRQHandler,//  127
    PD2_IRQHandler,//  128
    PD3_IRQHandler,//  129
    PD4_IRQHandler,//  130
    PD5_IRQHandler,//  131
    PD6_IRQHandler,//  132
    PD7_IRQHandler,//  133
    PD8_IRQHandler,//  134
    PD9_IRQHandler,//  135
    PD10_IRQHandler,//  136
    PD11_IRQHandler,//  137
    PD12_IRQHandler,//  138
    PD13_IRQHandler,//  139
    PD14_IRQHandler,//  140
    PD15_IRQHandler,//  141
    PE0_IRQHandler,//  142
    PE1_IRQHandler,//  143
    PE2_IRQHandler,//  144
    PE3_IRQHandler,//  145
    PE4_IRQHandler,//  146
    PE5_IRQHandler,//  147
    PE6_IRQHandler,//  148
    PE7_IRQHandler,//  149
    PE8_IRQHandler,//  150
    PE9_IRQHandler,//  151
    PE10_IRQHandler,//  152
    PE11_IRQHandler,//  153
    PE12_IRQHandler,//  154
    PE13_IRQHandler,//  155
    PE14_IRQHandler,//  156
    PE15_IRQHandler,//  157
    PF0_IRQHandler,//  158
    PF1_IRQHandler,//  159
    PF2_IRQHandler,//  160
    PF3_IRQHandler,//  161
    PF4_IRQHandler,//  162
    PF5_IRQHandler,//  163
    PF6_IRQHandler,//  164
    PF7_IRQHandler,//  165
    PF8_IRQHandler,//  166
    PF9_IRQHandler,//  167
    PF10_IRQHandler,//  168
    PF11_IRQHandler,//  169
    PF12_IRQHandler,//  170
    PF13_IRQHandler,//  171
    PF14_IRQHandler,//  172
    PF15_IRQHandler,//  173
    DMA_Active_0_IRQHandler,//  174
    DMA_Active_1_IRQHandler,//  175
    DMA_Active_2_IRQHandler,//  176
    DMA_Active_3_IRQHandler,//  177
    DMA_Done_0_IRQHandler,  //  178
    DMA_Done_1_IRQHandler,  //  179
    DMA_Done_2_IRQHandler,  //  180
    DMA_Done_3_IRQHandler,  //  181
    I2C0_MS_RX_IRQHandler,  //  182
    I2C0_MS_TX_IRQHandler,  //  183
    I2C0_SL_RX_IRQHandler,  //  184
    I2C0_SL_TX_IRQHandler,  //  185
    I2C1_MS_RX_IRQHandler,  //  186
    I2C1_MS_TX_IRQHandler,  //  187
    I2C1_SL_RX_IRQHandler,  //  188
    I2C1_SL_TX_IRQHandler,  //  189
    I2C2_MS_RX_IRQHandler,  //  190
    I2C2_MS_TX_IRQHandler,  //  191
    I2C2_SL_RX_IRQHandler,  //  192
    I2C2_SL_TX_IRQHandler,  //  193
    FPU_IRQHandler,          //  194
    TXEV_IRQHandler         //  195
};

