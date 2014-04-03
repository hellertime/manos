#include <manos.h>

int torgo_main(int, char**);

/*
 * Kernel entry point. For now it just launches the shell.
 */
int main(int argc, char** argv) {

#ifdef PLATFORM_K70CW
    mcgInit();
    sdramInit();
#endif

    return torgo_main(argc, argv);
}
