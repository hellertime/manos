#include <manos.h>
#include <arch/k70/derivative.h>

/**
 * struct Control - defines hardware control points for a timer
 * @timerScgc       timer status and gate control register
 * @timerSc         timer statuc and control register
 * @timerCount      timer count register
 * @timerMod        timer modulus ... wrap triggers interrupt
 * @timerMode       timer mode register
 * @timerSimOpt     timer option selection register
 * @timerClock      clock source
 * @timerPsd        pre scalar divider
 * @timerScgcMask   status and gate control register enable mask
 * @timerModeMask   mode selection mask
 * @timerSimOptMask clock source mask
 * @timerIRQ        interrupt 
 * @timerPriority   interrupt priority
 */
typedef struct Control {
    volatile uint32_t * const timerScgc;
    volatile uint32_t * const timerSc;
    volatile uint32_t * const timerCount;
    volatile uint32_t * const timerMod;
    volatile uint32_t * const timerMode;
    volatile uint32_t * const timerSimOpt;
    uint32_t                  timerScgcMask;
    uint32_t                  timerModeMask;
    uint32_t                  timerSimOptMask;
    uint32_t                  timerIRQ;
    uint32_t                  timerPriority;
} Control;

static Control k70Control[] = {
{   .timerScgc       = &SIM_SCGC6
,   .timerSc         = &FTM0_SC
,   .timerCount      = &FTM0_CNT
,   .timerMod        = &FTM0_MOD
,   .timerMode       = &FTM0_MODE
,   .timerSimOpt     = &SIM_SOPT4
,   .timerScgcMask   = SIM_SCGC6_FTM0_MASK
,   .timerModeMask   = FTM_MODE_WPDIS_MASK
,   .timerSimOptMask = SIM_SOPT4_FTM0CLKSEL_MASK
,   .timerIRQ        = NVIC_IRQ_FTM0
,   .timerPriority   = MANOS_ARCH_K70_TIMER_PRIORITY
}
};

extern TimerHW k70TimerHW;

Timer k70Timer[] = {
{   .regs  = &k70Control[0]
,   .name  = "k70FTM0"
,   .clock = FLEX_TIMER_CLOCK_SYSTEM
,   .psd   = 5 /* 1 << 5 */
,   .mod   = 1875
,   .hw    = &k70TimerHW
,   .next  = 0
}
};

static Timer* k70TimerHotplug(void) {
#ifdef PLATFORM_K70CW
    return k70Timer;
#else
    return NULL;
#endif
}

static void k70TimerStart(Timer* timer) {
    Control* ctrl  = timer->regs;
    *ctrl->timerSc = FTM_SC_TOIE_MASK          /* start with interrupts enabled on overflow */
                   | FTM_SC_CLKS(timer->clock) /* choose the provided clock source */
                   | FTM_SC_PS(timer->psd)
                   ;
}

static void k70TimerStop(Timer* timer) {
    Control* ctrl  = timer->regs;
    *ctrl->timerSc = FTM_SC_TOIE_MASK                   /* enable overflow interrupts */
                   | FTM_SC_CLKS(FLEX_TIMER_CLOCK_NONE) /* disable the counter by removing the clock source */
                   | FTM_SC_PS(timer->psd)              /* setup the provided prescalar divider */
                   ;
}

static void k70TimerReset(Timer* timer) {
    Control* ctrl     = timer->regs;
    *ctrl->timerCount = 0;
    *ctrl->timerMod   = timer->mod;
}

static void k70TimerDisable(Timer* timer) {
    Control* ctrl   = timer->regs;
    uint32_t sink   = *ctrl->timerSc;
    UNUSED(sink);
}

static void k70TimerPower(Timer* timer, int onoff) {
    Control* ctrl = timer->regs;
    if (onoff == 1) {
        *ctrl->timerScgc |= ctrl->timerScgcMask;

        k70TimerDisable(timer);

        *ctrl->timerMode   = ctrl->timerModeMask;
        *ctrl->timerSimOpt &= ~ctrl->timerSimOptMask;

        k70TimerStop(timer);
        k70TimerReset(timer);

        enableNvicIrq(ctrl->timerIRQ, ctrl->timerPriority);
    } /* not handling off */
}

TimerHW k70TimerHW = {
    .name    = "k70Timer"
,   .hotplug = k70TimerHotplug
,   .disable = k70TimerDisable
,   .power   = k70TimerPower
,   .reset   = k70TimerReset
,   .start   = k70TimerStart
,   .stop    = k70TimerStop
};
