#include <manos.h>

extern TimerHW niceTimerHW;

Timer niceTimer[] = {
{   .name   = "timeval"
,   .clock  = 0
,   .hw     = &niceTimerHW
,   .next   = 0
}
};

static Timer* niceTimerHotplug(void) {
#ifdef PLATFORM_NICE
    return niceTimer;
#else
    return NULL;
#endif
}

static void niceTimerDisable(Timer* timer) {
    UNUSED(timer);
}

static void niceTimerStart(Timer* timer) {
    UNUSED(timer);
}

static void niceTimerStop(Timer* timer) {
    UNUSED(timer);
}

static void niceTimerReset(Timer* timer) {
    UNUSED(timer);
}

static void niceTimerPower(Timer* timer, int onoff) {
    UNUSED(onoff);
    UNUSED(timer);
}

TimerHW niceTimerHW = {
    .name    = "niceTimer"
,   .hotplug = niceTimerHotplug
,   .disable = niceTimerDisable
,   .power   = niceTimerPower
,   .reset   = niceTimerReset
,   .start   = niceTimerStart
,   .stop    = niceTimerStop
};
