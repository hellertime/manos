#include <manos.h>

/**
 * pdbHandler() - programmable delay block handler
 */
void pdbHandler(void) {
    Timer* timer = hotpluggedTimers->next;
    if (timer) {
        timer->oneShotAction();
        timer->hw->stop(timer);
    }
}
