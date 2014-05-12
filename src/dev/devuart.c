#include <errno.h>
#include <manos.h>
#include <string.h>

static Uart* enableUart(Uart* uart) {
    uart->hw->enable(uart);

    if (uart->bits == 0)
        sysuartctl(uart, "l8");

    if (uart->baud == 0)
        sysuartctl(uart, "b9600");

    uart->enabled = 1;
    return uart;
}

static void disableUart(Uart* uart) {
    uart->hw->disable(uart);
}

static unsigned uartSNSCount = 0;
static StaticNS* uartSNS = NULL; /* to be populated at startup */

typedef enum {
    UartCtlFile
,   UartStatusFile
,   UartDataFile
} UartFileType;

static void resetUart(void) {
    Uart* uart = NULL;
    Uart* hpChain = NULL;

    unsigned hpCount = 0;
    for (unsigned i = 0; i < MANOS_MAXUART; i++) {
        if ((uart = uartHardwareTable[i]->hotplug()) == NULL)
            continue;

        if (hotpluggedUarts != NULL)
            hpChain->next = uart;
        else
            hotpluggedUarts = uart;

        hpCount++; /* count the uart added */

        for (hpChain = uart; hpChain->next != NULL; hpChain = hpChain->next)
            hpCount++; /* count the next in the uart chain */
    }

    uartSNSCount = 2 + (3 * hpCount); /* 3 files per Uart, plus dot, sentinel */
    uartSNS = syskmalloc(uartSNSCount * sizeof(StaticNS));
    
    StaticNS* sns = uartSNS;
    strcpy(sns->name, ".");
    sns->crumb.flags = CRUMB_ISDIR | CRUMB_ISSTATIC;
    sns->crumb.fid = MKSTATICNS_FID(STATICNS_SENTINEL, 0);
    sns->length = 0;
    sns->mode = 0555;
    sns->contents = 0;
    sns++;

    /* build a 'static' namespace, overload length and contents for our purposes on files */
    uart = hotpluggedUarts;
    for (unsigned i = 0; i < hpCount; i++) {
        strcpy(sns->name, uart->hw->name);
        sns->crumb.flags = CRUMB_ISFILE | CRUMB_ISSTATIC;
        sns->crumb.fid = MKSTATICNS_FID(0, (i * 3) + 1);
        sns->length = UartDataFile;
        sns->mode = 0666;
        sns->contents = (char*)uart; /* link back to hardware */
        sns++;
        fmtSprintf(sns->name, "%s_ctl", uart->hw->name);
        sns->crumb.flags = CRUMB_ISFILE | CRUMB_ISSTATIC;
        sns->crumb.fid = MKSTATICNS_FID(0, (i * 3) + 2);
        sns->length = UartCtlFile;
        sns->mode = 0666;
        sns->contents = (char*)uart; /* all files have the link */
        sns++;
        fmtSprintf(sns->name, "%s_status", uart->hw->name);
        sns->crumb.flags = CRUMB_ISFILE | CRUMB_ISSTATIC;
        sns->crumb.fid = MKSTATICNS_FID(0, (i * 3) + 3);
        sns->length = UartStatusFile;
        sns->mode = 0444;
        sns->contents = (char*)uart;
        sns++;

        uart = uart->next;
    }

    sns->name[0] = 0;
    sns->crumb.flags = CRUMB_ISSTATIC;
    sns->crumb.fid = MKSTATICNS_FID(STATICNS_SENTINEL, STATICNS_SENTINEL);
    sns->length = 0;
    sns->mode = 0;
    sns->contents = 0;
}

static Portal* attachUart(char *path) {
    Portal* p = attachDev(DEV_DEVUART, path);
    p->crumb = uartSNS->crumb;
    return p;
}

static NodeInfo* uartNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, uartSNS, d, ni);
}

static WalkTrail* walkUart(Portal* p, char **path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, uartNodeInfoFn);
}

static Portal* openUart(Portal* p, Caps caps) {
    return openDev(p, caps);
}

static void closeUart(Portal* p) {
    if (p->crumb.flags & CRUMB_ISDIR)
        return;

    NodeInfo ni;
    if (getNodeInfoStaticNS(p, uartSNS, WalkSelf, &ni) == NULL) {
        errno = ENODEV;
        return;
    }

    UartFileType t = ni.length; /* overloaded this field for private use */
    if (t == UartCtlFile || t == UartDataFile) {
        Uart* uart = (Uart*)ni.contents;
        disableUart(uart);
    }    
}

static ptrdiff_t readUart(Portal* p, void* buf, size_t size, Offset offset) {
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        return readStaticNS(p, uartSNS, buf, size, offset);
    }

    NodeInfo ni;
    if (getNodeInfoStaticNS(p, uartSNS, WalkSelf, &ni) == NULL) {
        errno = ENODEV;
        return -1;
    }
    size_t bytes = 0;
    Uart* uart = (Uart*)ni.contents;
    switch ((UartFileType)ni.length) {
    case UartDataFile:
        if (!uart->hw->getc) {
            errno = ENODEV;
            return -1;
        }
        while (bytes < size) {
            char c = uart->hw->getc(uart);
            switch (c) {
            case 0x03:
                sysprintln("%s: ctrl-C", uart->name);
                break;
            case 0x04:
                sysprintln("%s: ctrl-D", uart->name);
                break;
            case 0x1a:
                sysprintln("%s: ctrl-Z", uart->name);
                break;
            default:
                *((char*)buf + bytes) = c;
                bytes++;
                break;
            }
        }
        return bytes;
    default:
        errno = EINVAL;
        return -1;
    }
}

static ptrdiff_t writeUart(Portal* p, void* buf, size_t size, Offset offset) {
    UNUSED(offset);
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        errno = EPERM;
        return -1;
    }

    NodeInfo ni;
    if (getNodeInfoStaticNS(p, uartSNS, WalkSelf, &ni) == NULL) {
        errno = ENODEV;
        return -1;
    }

    size_t bytes = 0;
    Uart* uart = (Uart*)ni.contents;
    switch ((UartFileType)ni.length) {
    case UartDataFile:
        if (!uart->hw->putc) {
            errno = EPERM;
            return -1;
        }

        while (bytes < size) {
            char c = *((char*)buf + bytes);
            if (uart->console && c == '\n')
                uart->hw->putc(uart, '\r');
            uart->hw->putc(uart, c);
            bytes++;
        }
        return bytes;
    default:
        errno = EINVAL;
        return -1;
    }
}

/* TODO: This is incorrect since length is overloaded! */
static int getInfoUart(const Portal* p, NodeInfo *ni) {
    return getNodeInfoStaticNS(p, uartSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

static void powerUart(OnOff onoff) {
    for (Uart* uart = hotpluggedUarts; uart; uart = uart->next) {
        if(uart->hw->power)
            uart->hw->power(uart, onoff);
    }
}

static void initUart(void) {
    for (Uart* uart = hotpluggedUarts; uart; uart = uart->next) {
        if (uart->console)
            enableUart(uart);
    }
}

Dev devUart = {
    .id       = DEV_DEVUART
,   .name     = "uart"
,   .power    = powerUart
,   .init     = initUart
,   .reset    = resetUart
,   .shutdown = shutdownDev
,   .attach   = attachUart
,   .walk     = walkUart
,   .create   = createDev
,   .open     = openUart
,   .close    = closeUart
,   .remove   = removeDev
,   .getInfo  = getInfoUart
,   .setInfo  = setInfoDev
,   .read     = readUart
,   .write    = writeUart
};
