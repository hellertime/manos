#include <manos.h>
#include <string.h>
#include <inttypes.h>

extern void enterUserMode(void);

int torgo_main(int, char**);

extern uint32_t totalRAM;
extern size_t numChunkOffsets;
extern char* heap;
/*
 * Kernel entry point. For now it just launches the shell.
 */
int main(int argc, char** argv) {

#ifdef PLATFORM_K70CW
    mcgInit();
    sdramInit();
    svcInit(MANOS_ARCH_K70_SVC_INT_PRIORITY);
    schedInit(MANOS_ARCH_K70_SCHED_INT_PRIORITY);
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

    Proc* flist = kmalloc(MANOS_MAX_PROC * sizeof(*flist));
    Proc* p = flist;
    for (unsigned i = 0; i < MANOS_MAX_PROC; i++, p++) {
        INIT_LIST_HEAD(&p->nextFreelist);
        listAddAfter(&procFreelist, &p->nextFreelist);
    }

    sysprintln("Total System RAM: %" PRIu32 "", totalRAM);
    sysprintln(" # Chunk Offsets: %" PRIu32 "", numChunkOffsets);
    sysprintln("    Heap Address: 0x%.8" PRIx32 "", (uintptr_t)heap);

#ifdef PLATFORM_K70CW
    sysprintln("Entering User Mode...");
    enterUserMode();
#endif

    sysexecv("/bin/sh", 0);
    return -1; /* never returns */
}
