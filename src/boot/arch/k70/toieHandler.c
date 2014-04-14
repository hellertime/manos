#include <manos.h>

/**
 * toieHandler - timer overflow interrupt handler
 */
void toieHandler(void) {
    for (Timer* timer = hotpluggedTimers; timer; timer = timer->next) {
        timer->hw->disable(timer);
        timer->hw->start(timer);
        timer->timestamp.msecs++;
    }
}
