#include <manos.h>

int cmdFizzbuzz__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    int fizz = kopen("/dev/led/green", CAP_WRITE);
    int buzz = kopen("/dev/led/blue", CAP_WRITE);

    while (1) {
        uint64_t tick;
        int time = kopen("/dev/timers/k70Timer", CAP_READ);
        kread(time, &tick, sizeof tick);
        kclose(time);
        tick /= 1000; /* scale to seconds */
        if (tick % 15) {
            kwrite(fizz, "1", 1);
            kwrite(buzz, "1", 1);
        } else if (tick % 5 == 0) {
            kwrite(fizz, "0", 1);
            kwrite(buzz, "1", 1);
        } else if (tick % 3 == 0) {
            kwrite(buzz, "0", 1);
            kwrite(fizz, "1", 1);
        }
    }
}
