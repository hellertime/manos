#include <errno.h>
#include <manos.h>

/* assumes presence of the global device table */
DeviceIndex fromDeviceId(DeviceId id) {
    for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
        if (deviceTable[i]->id == id)
            return i;
    }

    errno = ENODEV;
    return -1;
}

DeviceId toDeviceId(DeviceIndex device) {
    if (device < 0 || (unsigned)device > COUNT_OF(deviceTable)) {
        errno = EINVAL;
        return -1;
    }

    return deviceTable[device]->id;
}
