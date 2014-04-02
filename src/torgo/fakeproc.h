/* * Until we have a real process model, this file will holds the bits an pieces to fake it
 */
extern Dev devRoot;
extern Dev devLed;
extern Dev devSwpb;

Dev* deviceTable[MANOS_MAXDEV] = {
    &devRoot
,   &devLed
,   &devSwpb
};

Portal* descriptorTable[MANOS_MAXFD] = {0};

static void initDeviceTable(void) {
    for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
        deviceTable[i]->init();
    }
    UNUSED(descriptorTable);
}

Portal* slash; /* the 'process' root */
Portal* dot;   /* the 'process' cwd */

#define HAS_FAKEPROC
