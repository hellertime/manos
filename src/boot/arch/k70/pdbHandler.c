#include <manos.h>
#include <manos/list.h>

extern long long pdbInterruptCount;

extern void __kfree(void*);
/**
 * pdbHandler() - programmable delay block handler
 */
void pdbHandler(void) {
    ATOMIC(pdbInterruptCount++);

    AlarmChain* iter;
    AlarmChain* save;
    Timer* timer = hotpluggedTimers->next;
    if (timer) {
        timer->hw->clear(timer);

        if (listIsEmpty(&timer->alarms))
            return;

        timer->hw->stop(timer);
        enterCriticalRegion();
        uint64_t now = systime;
        LIST_FOR_EACH_ENTRY_SAFE(iter, save, &timer->alarms, next) {
            if (iter->wakeTime <= now) {
                syspostsignal(iter->pid, SigAlarm);
                listUnlink(&iter->next);
                __kfree(iter);
            }
        }
        leaveCriticalRegion();
        timer->hw->start(timer);
    }
}
