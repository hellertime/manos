#include <manos.h>

int cmdLsmem__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    kmallocDump();
    return 0;
}
