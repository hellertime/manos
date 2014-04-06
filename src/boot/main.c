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
    svcInit(7);
#endif

    /*
     * This cannot be allocated on the heap since the allocator
     * needs 'u' to get the pid to stick in the chunk tag.
     * And wel...
     */
    Proc firstProc = {
        .pid   = 0
    };

    u = &firstProc;


    for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
        deviceTable[i]->init();
        deviceTable[i]->reset();
        deviceTable[i]->power(1);
    }
  
#if PLATFORM_K70CW
    k70Console();
#elif PLATFORM_NICE
    niceConsole();
#endif

    sysprintln("Initializing first user...");

    Proc* uptr = kmallocz(sizeof *uptr);
    if (!uptr) {
        sysprintln("PANIC! Cannot create first user");
        return 1;
    }

    memcpy(u, uptr, sizeof *uptr);

    sysprintln("Creating namespace...");

    u->slash = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
    u->dot   = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");

    sysprintln("Total System RAM: %" PRIu32 "", totalRAM);
    sysprintln(" # Chunk Offsets: %" PRIu32 "", numChunkOffsets);
    sysprintln("    Heap Address: 0x%.8" PRIx32 "", (uintptr_t)heap);

#ifdef PLATFORM_K70CW
    u->tty = kopen("/dev/uart/k70Uart", CAP_READWRITE);
#else
    u->tty = kopen("/dev/uart/stdio", CAP_READWRITE);
#endif

    if (u->tty == -1) {
        sysprintln("ERROR: Cannot open tty");
    }

#ifdef PLATFORM_K70CW
    sysprintln("Entering User Mode...");
    enterUserMode();
#endif

    fprintln(u->tty, "\nMANOS: Welcome Master...\n");
    return torgo_main(argc, argv);
}
