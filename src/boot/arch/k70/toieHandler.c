#include <manos.h>

/**
 * toieHandler - timer overflow interrupt handler
 */
void toieHandler(void) {
    Timer* timer = hotpluggedTimers;
    if (timer) {
        timer->hw->disable(timer);
        timer->hw->start(timer);
        DISABLE_INTERRUPTS();
        timer->timestamp.msecs++;
        ENABLE_INTERRUPTS();
    }
}
