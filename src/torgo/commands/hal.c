#include <manos.h>
#include <stdint.h>

#include "hal2000.ascii"

int cmdHal__Main(int argc, char * const argv[]) {
    char blitbuf[LCD_SDRAM_SIZE] = {0};

    char* p = &blitbuf[0];
    size_t bytes = 0;
    for (uint32_t i = 0; i < COUNT_OF(hal); i += 2) {
        uint32_t octet = hal[i];
        uint32_t run = hal[i + 1];

        while (run--) {
            *p++ = (uint8_t)octet;
            bytes++;
        }
    }

    int fd = kopen("/dev/lcd/blit", CAP_WRITE);
    kwrite(fd, blitbuf, bytes);
    kclose(fd);

    UNUSED(argv);
    UNUSED(argc);
    return 0;
}
