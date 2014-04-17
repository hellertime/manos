/*
 *    kinetis_sysinit.c - Default init routines for P3
 *                     		Kinetis ARM systems
 *    Copyright © 2012 Freescale semiConductor Inc. All Rights Reserved.
 */
 
#include "kinetis_sysinit.h"
#include "arch/k70/derivative.h"

extern void svcHandler(void);
extern void hardFaultHandler(void);
extern void toieHandler(void);
extern void k70UartInterrupt(void);

/**
 **===========================================================================
 **  External declarations
 **===========================================================================
 */
#if __cplusplus
extern "C" {
#endif
extern uint32_t __vector_table[];
extern unsigned long _estack;
extern void __thumb_startup(void);
#if __cplusplus
}
#endif

/**
 **===========================================================================
 **  Default interrupt handler
 **===========================================================================
 */
void Default_Handler()
{
	__asm("bkpt");
}

/**
 **===========================================================================
 **  Reset handler
 **===========================================================================
 */
void __init_hardware()
{
	SCB_VTOR = (uint32_t)__vector_table; /* Set the interrupt vector table position */
	/*
		Disable the Watchdog because it may reset the core before entering main().
		There are 2 unlock words which shall be provided in sequence before
		accessing the control register.
	*/
	WDOG_UNLOCK = KINETIS_WDOG_UNLOCK_SEQ_1;
	WDOG_UNLOCK = KINETIS_WDOG_UNLOCK_SEQ_2;
	WDOG_STCTRLH = KINETIS_WDOG_DISABLED_CTRL;
}

/* Weak definitions of handlers point to Default_Handler if not implemented */
void NMI_Handler() __attribute__ ((weak, alias("Default_Handler")));
/* void HardFault_Handler() __attribute__ ((weak, alias("Default_Handler"))); */
void MemManage_Handler() __attribute__ ((weak, alias("Default_Handler")));
void BusFault_Handler() __attribute__ ((weak, alias("Default_Handler")));
void UsageFault_Handler() __attribute__ ((weak, alias("Default_Handler")));
/* void SVC_Handler() __attribute__ ((weak, alias("Default_Handler"))); */
void DebugMonitor_Handler() __attribute__ ((weak, alias("Default_Handler")));
void PendSV_Handler() __attribute__ ((weak, alias("Default_Handler")));
void SysTick_Handler() __attribute__ ((weak, alias("Default_Handler")));


/* The Interrupt Vector Table */
void (* const InterruptVector[])() __attribute__ ((section(".vectortable"))) = {
    /* Processor exceptions */
    (void(*)(void)) &_estack,
    __thumb_startup,
    NMI_Handler, 
    hardFaultHandler, 
    MemManage_Handler, 
    BusFault_Handler,
    UsageFault_Handler, 
    0, 
    0, 
    0, 
    0, 
    svcHandler, 
    DebugMonitor_Handler, 
    0,
    PendSV_Handler, 
    SysTick_Handler,

    /* Interrupts */
    Default_Handler,	/* IRQ0 */
    Default_Handler,	/* IRQ1 */
    Default_Handler,	/* IRQ2 */
    Default_Handler,	/* IRQ3 */
    Default_Handler,	/* IRQ4 */
    Default_Handler,	/* IRQ5 */
    Default_Handler,	/* IRQ6 */
    Default_Handler,	/* IRQ7 */
    Default_Handler,	/* IRQ8 */
    Default_Handler,	/* IRQ9 */
    Default_Handler,	/* IRQ10 */
    Default_Handler,	/* IRQ11 */
    Default_Handler,	/* IRQ12 */
    Default_Handler,	/* IRQ13 */
    Default_Handler,	/* IRQ14 */
    Default_Handler,	/* IRQ15 */
    Default_Handler,	/* IRQ16 */
    Default_Handler,	/* IRQ17 */
    Default_Handler,	/* IRQ18 */
    Default_Handler,    /* IRQ19 */
    Default_Handler,	/* IRQ20 */
    Default_Handler,	/* IRQ21 */
    Default_Handler,	/* IRQ22 */
    Default_Handler,	/* IRQ23 */
    Default_Handler,	/* IRQ24 */
    Default_Handler,	/* IRQ25 */
    Default_Handler,	/* IRQ26 */
    Default_Handler,	/* IRQ27 */
    Default_Handler,	/* IRQ28 */
    Default_Handler,	/* IRQ29 */
    Default_Handler,	/* IRQ30 */
    Default_Handler,	/* IRQ31 */
    Default_Handler,	/* IRQ32 */
    Default_Handler,	/* IRQ33 */
    Default_Handler,	/* IRQ34 */
    Default_Handler,	/* IRQ35 */
    Default_Handler,	/* IRQ36 */
    Default_Handler,	/* IRQ37 */
    Default_Handler,	/* IRQ38 */
    Default_Handler,	/* IRQ39 */
    Default_Handler,	/* IRQ40 */
    Default_Handler,	/* IRQ41 */
    Default_Handler,	/* IRQ42 */
    Default_Handler,	/* IRQ43 */
    Default_Handler,	/* IRQ44 */
    Default_Handler,	/* IRQ45 */
    Default_Handler,	/* IRQ46 */
    Default_Handler,	/* IRQ47 */
    Default_Handler,	/* IRQ48 */
    k70UartInterrupt,	/* IRQ49 */
    Default_Handler,	/* IRQ50 */
    Default_Handler,	/* IRQ51 */
    Default_Handler,	/* IRQ52 */
    Default_Handler,	/* IRQ53 */
    Default_Handler,	/* IRQ54 */
    Default_Handler,	/* IRQ55 */
    Default_Handler,	/* IRQ56 */
    Default_Handler,	/* IRQ57 */
    Default_Handler,	/* IRQ58 */
    Default_Handler,	/* IRQ59 */
    Default_Handler,	/* IRQ60 */
    Default_Handler,	/* IRQ61 */
    toieHandler,	/* IRQ62 */
    Default_Handler,	/* IRQ63 */
    Default_Handler,	/* IRQ64 */
    Default_Handler,	/* IRQ65 */
    Default_Handler,	/* IRQ66 */
    Default_Handler,	/* IRQ67 */
    Default_Handler,	/* IRQ68 */
    Default_Handler,	/* IRQ69 */
    Default_Handler,	/* IRQ70 */
    Default_Handler,	/* IRQ71 */
    Default_Handler,	/* IRQ72 */
    Default_Handler,	/* IRQ73 */
    Default_Handler,	/* IRQ74 */
    Default_Handler,	/* IRQ75 */
    Default_Handler,	/* IRQ76 */
    Default_Handler,	/* IRQ77 */
    Default_Handler,	/* IRQ78 */
    Default_Handler,	/* IRQ79 */
    Default_Handler,	/* IRQ80 */
    Default_Handler,	/* IRQ81 */
    Default_Handler,	/* IRQ82 */
    Default_Handler,	/* IRQ83 */
    Default_Handler,	/* IRQ84 */
    Default_Handler,	/* IRQ85 */
    Default_Handler,	/* IRQ86 */
    Default_Handler,	/* IRQ87 */
    Default_Handler,	/* IRQ88 */
    Default_Handler,	/* IRQ89 */
    Default_Handler,	/* IRQ90 */
    Default_Handler,	/* IRQ91 */
    Default_Handler,	/* IRQ92 */	
    Default_Handler,	/* IRQ93 */
    Default_Handler,	/* IRQ94 */
    Default_Handler,	/* IRQ95 */
    Default_Handler,	/* IRQ96 */
    Default_Handler,	/* IRQ97 */
    Default_Handler,	/* IRQ98 */
    Default_Handler,	/* IRQ99 */
    Default_Handler,	/* IRQ100 */
    Default_Handler,	/* IRQ101 */	
    Default_Handler,	/* IRQ102 */
    Default_Handler,	/* IRQ103 */
    Default_Handler,	/* IRQ104 */
    Default_Handler,	/* IRQ105 */
};
