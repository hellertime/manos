#ifndef MANOS_ARCH_MK70F12_H
#define MANOS_ARCH_MK70F12_H

#ifdef PLATFORM_K70CW

#ifndef ENOBUFS
#define ENOBUFS EINVAL
#endif

#ifndef ENODATA
#define ENODATA EINVAL
#endif

#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE EINVAL
#endif

#ifndef PRIu32
#define PRIu32 "lu"
#endif

#ifndef PRIx32
#define PRIx32 "lx"
#endif

#ifndef PRIuPTR
#define PRIuPTR PRIu32
#endif

#ifndef PRIxPTR
#define PRIxPTR PRIx32
#endif

#ifndef intptr_t
typedef int32_t intptr_t;
#endif

#ifndef uintptr_t
typedef uint32_t uintptr_t;
#endif

void mcgInit(void);
void sdramInit(void);

#define SDRAM_START 0x8007f800 /* 0x80000000 + LCD_SDRAM_SIZE */

#endif /* PLATFORM_K70CW */

#define BASE_RAM 0x80000000

#define LCD_XSIZE 480
#define LCD_YSIZE 272
#define LCD_BPP   4
#define LCD_SDRAM_START BASE_RAM
#define LCD_SDRAM_SIZE 522240    /* LCD_XSIZE * LCD_YSIZE * LCD_BPP */

/* NVIC IRQ constants */
#define NVIC_IRQ_DMA0       0
#define NVIC_IRQ_DMA1       1
#define NVIC_IRQ_DMA2       2
#define NVIC_IRQ_DMA3       3
#define NVIC_IRQ_DMA4       4
#define NVIC_IRQ_DMA5       5
#define NVIC_IRQ_DMA6       6
#define NVIC_IRQ_DMA7       7
#define NVIC_IRQ_DMA8       8
#define NVIC_IRQ_DMA9       9
#define NVIC_IRQ_DMA10      10
#define NVIC_IRQ_DMA11      11
#define NVIC_IRQ_DMA12      12
#define NVIC_IRQ_DMA13      13
#define NVIC_IRQ_DMA14      14
#define NVIC_IRQ_DMA15      15
#define NVIC_IRQ_DMA_ERROR  16
#define NVIC_IRQ_MCM        17
#define NVIC_IRQ_FLASH_CC   18
#define NVIC_IRQ_FLASH_RC   19
#define NVIC_IRQ_LV_WARN    20
#define NVIC_IRQ_LLWAKE     21
#define NVIC_IRQ_WDOG       22
#define NVIC_IRQ_RNG        23
#define NVIC_IRQ_I2C0       24
#define NVIC_IRQ_I2C1       25
#define NVIC_IRQ_SPI0       26
#define NVIC_IRQ_SPI1       27
#define NVIC_IRQ_SPI2       28
#define NVIC_IRQ_CAN0_MB    29
#define NVIC_IRQ_CAN0_BO    30
#define NVIC_IRQ_CAN0_ERR   31
#define NVIC_IRQ_CAN0_TXW   32
#define NVIC_IRQ_CAN0_RXW   33
#define NVIC_IRQ_CAN0_WAKE  34
#define NVIC_IRQ_I2S0_TX    35
#define NVIC_IRQ_I2S0_RX    36
#define NVIC_IRQ_CAN1_MB    37
#define NVIC_IRQ_CAN1_BO    38
#define NVIC_IRQ_CAN1_ERR   39
#define NVIC_IRQ_CAN1_TXW   40
#define NVIC_IRQ_CAN1_RXW   41
#define NVIC_IRQ_CAN1_WAKE  42
#define NVIC_IRQ_UNUSED     43
#define NVIC_IRQ_UART0_LON  44
#define NVIC_IRQ_UART0_STAT 45
#define NVIC_IRQ_UART0_ERR  46
#define NVIC_IRQ_UART1_STAT 47
#define NVIC_IRQ_UART1_ERR  48
#define NVIC_IRQ_UART2_STAT 49
#define NVIC_IRQ_UART2_ERR  50
#define NVIC_IRQ_UART3_STAT 51
#define NVIC_IRQ_UART3_ERR  52
#define NVIC_IRQ_UART4_STAT 53
#define NVIC_IRQ_UART4_ERR  54
#define NVIC_IRQ_UART5_STAT 55
#define NVIC_IRQ_UART5_ERR  56
#define NVIC_IRQ_ADC0       57
#define NVIC_IRQ_ADC1       58
#define NVIC_IRQ_CMP0       59
#define NVIC_IRQ_CMP1       60
#define NVIC_IRQ_CMP2       61
#define NVIC_IRQ_FTM0       62
#define NVIC_IRQ_FTM1       63
#define NVIC_IRQ_FTM2       64
#define NVIC_IRQ_CMT        65
#define NVIC_IRQ_RTC_ALARM  66
#define NVIC_IRQ_RTC_SEC    67
#define NVIC_IRQ_PIT_CHAN0  68
#define NVIC_IRQ_PIT_CHAN1  69
#define NVIC_IRQ_PIT_CHAN2  70
#define NVIC_IRQ_PIT_CHAN3  71
#define NVIC_IRQ_PDB        72
#define NVIC_IRQ_USB_OTG    73
#define NVIC_IRQ_USB_CHARGE 74
#define NVIC_IRQ_ETH_1588   75
#define NVIC_IRQ_ETH_TX     76
#define NVIC_IRQ_ETH_RX     77
#define NVIC_IRQ_ETH_ERR    78
#define NVIC_IRQ_UNUSED2    79
#define NVIC_IRQ_SDHC       80
#define NVIC_IRQ_DAC0       81
#define NVIC_IRQ_DAC1       82
#define NVIC_IRQ_TSI        83
#define NVIC_IRQ_MCG        84
#define NVIC_IRQ_LP_TIMER   85
#define NVIC_IRQ_UNUSED3    86
#define NVIC_IRQ_PCM_PORTA  87
#define NVIC_IRQ_PCM_PORTB  88
#define NVIC_IRQ_PCM_PORTC  89
#define NVIC_IRQ_PCM_PORTD  90
#define NVIC_IRQ_PCM_PORTE  91
#define NVIC_IRQ_PCM_PORTF  92
#define NVIC_IRQ_DDR        93
#define NVIC_IRQ_SWI        94
#define NVIC_IRQ_NFC        95
#define NVIC_IRQ_USB_HS     96
#define NVIC_IRQ_LCD        97
#define NVIC_IRQ_CMP3       98
#define NVIC_IRQ_TAMPER     99
#define NVIC_IRQ_UNUSED4    100
#define NVIC_IRQ_FTM3       101
#define NVIC_IRQ_ADC2       102
#define NVIC_IRQ_ADC3       103
#define NVIC_IRQ_I2S1_TX    104
#define NVIC_IRQ_I2S1_RX    105
#define NVIC_IRQ_MAX        NVIC_IRQ_I2S1_RX

#define NVIC_IPR_REGISTER_BITS 4
#define NVIC_IPR_MAX_PRIORITY  ((1 << NVIC_IPR_REGISTER_BITS) - 1)

#define FLEX_TIMER_CLOCK_NONE   0
#define FLEX_TIMER_CLOCK_SYSTEM 1
#define FLEX_TIMER_CLOCK_FIXED  2
#define FLEX_TIMER_CLOCK_EXTERN 3

#define MANOS_ARCH_K70_SVC_INT_PRIORITY 15
#define MANOS_ARCH_K70_TIMER_PRIORITY   14

#endif /* ! MANOS_ARCH_MK70F12_H */
