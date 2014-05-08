#include <manos.h>
#include <manos/list.h>
#include <string.h>
#include <inttypes.h>

#include <torgo/commands.h>

extern Proc* schedProc(Cmd, int, char * const []);
extern void enterUserMode(void);

extern uint32_t totalRAM;
extern size_t numChunkOffsets;
extern char* heap;
/*
 * Kernel entry point. For now it just launches the shell.
 */
int main(int argc, char** argv) {
    INIT_LIST_HEAD(&procFreelist);
    INIT_LIST_HEAD(&procRunQ);
    INIT_LOCK(&freelistLock);
    INIT_REF(&nextPid);

#ifdef PLATFORM_K70CW
    mcgInit();
    sdramInit();
    svcInit(MANOS_ARCH_K70_SVC_INT_PRIORITY);
#endif

    for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
        deviceTable[i]->reset();
        deviceTable[i]->power(1);
        deviceTable[i]->init();
    }
  
#if PLATFORM_K70CW
    k70Console();
#elif PLATFORM_NICE
    niceConsole();
#endif

    sysprintln("Allocating free list...");

    Proc* flist = syskmalloc0(MANOS_MAXPROC * sizeof(*flist));
    Proc* p = flist;
    for (unsigned i = 0; i < MANOS_MAXPROC; i++, p++) {
        INIT_LIST_HEAD(&p->nextFreelist);
        listAddAfter(&procFreelist, &p->nextFreelist);
    }

    sysprintln("Total System RAM: %" PRIu32 "", totalRAM);
    sysprintln(" # Chunk Offsets: %" PRIu32 "", numChunkOffsets);
    sysprintln("    Heap Address: 0x%.8" PRIx32 "", (uintptr_t)heap);

    /* OK. Still in supervisor mode */
    schedProc(torgo_main, 0, NULL);
    /* schedProc disables interrupts before it returns */
    ENABLE_INTERRUPTS();
#ifdef PLATFORM_K70CW
    schedInit(50, MANOS_ARCH_K70_SCHED_INT_PRIORITY);
    sysprintln("Entering User Mode...");
    enterUserMode();
#endif

    UNUSED(argc);
    UNUSED(argv);
    return -1; /* never returns */
}


