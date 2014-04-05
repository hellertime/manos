#include <manos.h>
#include <stdio.h>
#include <string.h>

int torgo_main(int, char**);

/*
 * Kernel entry point. For now it just launches the shell.
 */
int main(int argc, char** argv) {

#ifdef PLATFORM_K70CW
    mcgInit();
    sdramInit();
    k70Console();
#elif PLATFORM_NICE
    niceConsole();
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

    Proc* uptr = kmallocz(sizeof *uptr);
    if (!uptr) {
        puts("PANIC! Cannot create first user\n");
        return 1;
    }

    memcpy(u, uptr, sizeof *uptr);

    u->slash = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
    u->dot   = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
 
    for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
        deviceTable[i]->init();
        deviceTable[i]->reset();
    }
  
#if PLATFORM_K70CW
    setvbuf(stdin, NULL, _IONBF, 0);
#endif
    return torgo_main(argc, argv);
}
