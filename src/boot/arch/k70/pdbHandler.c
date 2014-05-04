#include <manos.h>

extern long long pdbInterruptCount;

/**
 * pdbHandler() - programmable delay block handler
 */
void pdbHandler(void) {
    ATOMIC(pdbInterruptCount++);

    Timer* timer = hotpluggedTimers->next;
    if (timer) {
        timer->hw->clear(timer);
        if (timer->oneShotAction)
            timer->oneShotAction();
        timer->oneShotAction = 0;
    }
}
