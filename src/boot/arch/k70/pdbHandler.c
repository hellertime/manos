#include <manos.h>
#include <manos/list.h>

extern long long pdbInterruptCount;

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

        timer->hw->stop();
        enterCriticalRegion();
        int fd = sysopen("/dev/timer/k70Timer", CAP_READ);
        uint64_t now;
        sysread(fd, &now, sizeof now);
        sysclose(fd);
        LIST_FOR_EACH_ENTRY_SAFE(iter, save, &timer->alarms, next) {
            if (iter->wakeTime <= now) {
                syspostsignal(iter->pid, SigAlarm);
                listUnlink(&iter->next);
                syskfree(iter);
            }
        }
        leaveCriticalRegion();
        timer->hw->start();
    }
}
