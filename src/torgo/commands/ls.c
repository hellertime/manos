#include <manos.h>

int cmdLs__Main(int argc, char * const argv[]) {
    char* path = ".";
    if (argc > 1)
        path = argv[1];

    NodeInfo* ni;
    int fd = kopen(path, CAP_READ);
    int n;
    while ((n = dirread(fd, &ni)) > 0) {
        for (int i = 0; i < n; i++)
            sysprintln(ni[i].name);

        kfree(ni);
    }

    kclose(fd);

    return 0;
}
