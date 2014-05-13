#include <errno.h>
#include <manos.h>
#include <inttypes.h>

#include <arch/k70/derivative.h>

typedef struct Control {
    UART_MemMapPtr mmap;
    volatile uint32_t * const portScgc;
    uint32_t                  portScgcMask;
    volatile uint32_t * const uartScgc;
    uint32_t                  uartScgcMask;
    volatile uint32_t * const uartPortTxPin;
    volatile uint32_t * const uartPortRxPin; 
    uint32_t                  uartIRQ;
    uint32_t                  uartPriority;
    uint32_t                  uartInQDepth;
    uint32_t                  uartOutQDepth;
} Control;

static Control k70Control[] = {
{    .mmap          = UART2_BASE_PTR
,    .portScgc      = &SIM_SCGC5
,    .portScgcMask  = SIM_SCGC5_PORTE_MASK
,    .uartScgc      = &SIM_SCGC4
,    .uartScgcMask  = SIM_SCGC4_UART2_MASK
,    .uartPortTxPin = &PORTE_PCR16
,    .uartPortRxPin = &PORTE_PCR17
,    .uartIRQ       = NVIC_IRQ_UART2_STAT
,    .uartPriority  = MANOS_ARCH_K70_UART2_PRIORITY
,    .uartInQDepth  = 128
,    .uartOutQDepth = 128
}
};

extern UartHW k70UartHW;

Uart k70Uart[] = {
{    .regs    = &k70Control[0]
,    .name    = "k70Uart2"
,    .clock   = 60000000   /* Hz */
,    .hw      = &k70UartHW
,    .console = 1
,    .next    = 0
},
};

static Uart* k70UartHotplug(void) {
#ifdef PLATFORM_K70CW
    return k70Uart;
#else
    return NULL;
#endif
}

static void k70UartPower(Uart* uart, int onoff) {
    Control* ctrl = uart->regs;
    if (onoff == 1) {
        uart->inQ  = newFifoQ(ctrl->uartInQDepth);
        uart->outQ = newFifoQ(ctrl->uartOutQDepth);
        *ctrl->portScgc |= ctrl->portScgcMask;
        *ctrl->uartScgc |= ctrl->uartScgcMask;
        *ctrl->uartPortTxPin = PORT_PCR_MUX(0x3);
        *ctrl->uartPortRxPin = PORT_PCR_MUX(0x3);
    } /* only handle on at the moment */
}

static void k70UartEnable(Uart* uart) {
    Control* ctrl = uart->regs;
    UART_C2_REG(ctrl->mmap) |= (UART_C2_TE_MASK | UART_C2_RE_MASK | UART_C2_RIE_MASK);
    enableNvicIrq(ctrl->uartIRQ, ctrl->uartPriority);
}

static void k70UartDisable(Uart* uart) {
    Control* ctrl = uart->regs;
    UART_C2_REG(ctrl->mmap) &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
}

static int k70UartBits(Uart* uart, int bits) {
    Control* ctrl = uart->regs;

    switch (bits) {
    case 8:
        UART_C1_REG(ctrl->mmap) = 0;
        break;
    default:
        errno = EINVAL;
        return -1; /* only support 8-bits at the moment */
    }

    uart->bits = bits; /* save the new setting */
    return 0;
}

static int k70UartBaud(Uart* uart, unsigned baud) {
    Control* ctrl = uart->regs;

    if (uart->clock == 0) {
        errno = EINVAL;
        return -1;
    }

    uint16_t newSBR = (uint16_t)(uart->clock / baud / 16);              /* See Ref manual 54.4.4/1980 */
    uint8_t saveBDH = UART_BDH_REG(ctrl->mmap) & ~(UART_BDH_SBR(0x1f)); /* Mask off old SB before saving BDH */

    /* Change not in effect until BDL is written so write High, then Low */
    UART_BDH_REG(ctrl->mmap) = saveBDH | UART_BDH_SBR(((newSBR & 1000) >> 8));
    UART_BDL_REG(ctrl->mmap) = (uint8_t)(newSBR & UART_BDL_SBR_MASK);

    /* Compute a potential fractional divider to fine-tune the baud */
    uint16_t newBRFA = ((((uint32_t)(uart->clock / 1000)* 32000) / (baud * 16)) - (newSBR * 32));
    uint8_t   saveC4 = UART_C4_REG(ctrl->mmap) & ~(UART_C4_BRFA(0x1f)); /* Mask off old BRFA before saving C4 */

    UART_C4_REG(ctrl->mmap) = saveC4 | UART_C4_BRFA(newBRFA);

    uart->baud = baud;
    return 0;
}

#if 0
static char k70UartGetc(Uart* uart) {
    Control* ctrl = uart->regs;
    while(!(UART_S1_REG(ctrl->mmap) & UART_S1_RDRF_MASK))
        /* nanosleep */;

    return UART_D_REG(ctrl->mmap);
}

static void k70UartPutc(Uart* uart, char c) {
    Control* ctrl = uart->regs;
    while(!(UART_S1_REG(ctrl->mmap) & UART_S1_TDRE_MASK))
        /* nanosleep */;

    UART_D_REG(ctrl->mmap) = (uint8_t)c;
}
#endif

static char k70UartGetc(Uart* uart) {
    char c;
    enterCriticalRegion();
    while (! dequeueFifoQ(uart->inQ, &c)) {
        leaveCriticalRegion();
        enterCriticalRegion();
    }
    leaveCriticalRegion();
    return c;
}

static void k70UartPutc(Uart* uart, char c) {
    Control* ctrl = uart->regs;

    enterCriticalRegion();
    while (! enqueueFifoQ(uart->outQ, c)) {
        leaveCriticalRegion();
        enterCriticalRegion();
    }
    UART_C2_REG(ctrl->mmap) |= UART_C2_TIE_MASK;
    leaveCriticalRegion();
}

UartHW k70UartHW = {
    .name    = "k70Uart"
,   .hotplug = k70UartHotplug
,   .enable  = k70UartEnable
,   .disable = k70UartDisable
,   .power   = k70UartPower
,   .baud    = k70UartBaud
,   .bits    = k70UartBits
,   .getc    = k70UartGetc
,   .putc    = k70UartPutc
};

void k70UartInterrupt(void) {
    Uart* uart    = &k70Uart[0];
    Control* ctrl = uart->regs;

    uint32_t status = UART2_S1;

    int tdre = status & UART_S1_TDRE_MASK;
    int rdrf = status & UART_S1_RDRF_MASK;
    if (! (tdre || rdrf)) {
        sysprintln("k70Uart() weird uart interrupt 0x%" PRIx32 "", status);
    }

    if (UART_C2_REG(ctrl->mmap) & UART_C2_TIE_MASK && tdre) {
        char c;
        if (dequeueFifoQ(uart->outQ, &c))
            UART_D_REG(ctrl->mmap) = c;

        if (uart->outQ->isEmpty)
            UART_C2_REG(ctrl->mmap) &= ~UART_C2_TIE_MASK;
    }

    if (UART_C2_REG(ctrl->mmap) & UART_C2_RIE_MASK && rdrf) {
        char c = UART_D_REG(ctrl->mmap);
        switch (c) {
        case 0x03:
            sysprintln("%s: ctrl-C", uart->name);
            break;
        case 0x04:
            sysprintln("%s: ctrl-D", uart->name);
            break;
        case 0x1a:
            if (rp) {
                sysprintln("post() SigStop. %d", rp->pid);
                syspostsignal(rp->pid, SigStop);
            }
            break;
        default:
            enqueueFifoQ(uart->inQ, c);
            break;
        }
    }
}
