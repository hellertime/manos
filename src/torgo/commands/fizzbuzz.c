#include <manos.h>

int cmdFizzbuzz__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    int fizz = kopen("/dev/led/green", CAP_WRITE);
    int buzz = kopen("/dev/led/blue", CAP_WRITE);

    for (long i = 0; ; i++) {
        if (i % 500 == 0 && i % 300 == 0) {
            kwrite(fizz, "1", 1);
            kwrite(buzz, "1", 1);
        } else if (i % 500 == 0) {
            kwrite(fizz, "0", 1);
            kwrite(buzz, "1", 1);
        } else if (i % 300 == 0) {
            kwrite(buzz, "0", 1);
            kwrite(fizz, "1", 1);
        }
    }
}
