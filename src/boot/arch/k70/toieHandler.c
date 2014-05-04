#include <manos.h>

extern long long timerInterruptCount;

/**
 * toieHandler - timer overflow interrupt handler
 */
void toieHandler(void) {
    ATOMIC(timerInterruptCount++);

    Timer* timer = hotpluggedTimers;
    if (timer) {
        timer->hw->disable(timer);
        timer->hw->start(timer);
        ATOMIC(timer->timestamp.msecs++);
    }
}
