#include <manos.h>

/**
 * pdbHandler() - programmable delay block handler
 */
void pdbHandler(void) {
    Timer* timer = hotpluggedTimers->next;
    if (timer) {
        if (timer->oneShotAction)
            timer->oneShotAction();
        timer->oneShotAction = 0;
        timer->hw->stop(timer);
    }
}
