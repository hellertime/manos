#include <manos.h>
#include <stdint.h>

#define HAL_ASCII_SIZE 510476
#define HAL_INT_SIZE 170159

#include "hal2000.ascii"

int cmdHal__Main(int argc, char * const argv[]) {
    uint32_t* image = kmalloc(HAL_INT_SIZE * (sizeof *image));
    if (!image) {
        fprintln(u->tty, "Unable to allocate memory for image conversion");
        return 1;
    }

    for (unsigned i = 0, j = 0; i < HAL_ASCII_SIZE; i+= 3, j++) {
        uint32_t pixel = *(uint32_t*)&hal[i]; /* this stores 4 bytes, but hal only holds 3 octets per pixel */
        pixel >>= 8; /* so shift out the last pixel value */
        image[j] = pixel;
    }

    int fd = kopen("/dev/lcd/blit", CAP_WRITE);
    kwrite(fd, image, HAL_INT_SIZE * (sizeof *image));
    kclose(fd);

    kfree(image);
    UNUSED(argv);
    UNUSED(argc);
    return 0;
}
