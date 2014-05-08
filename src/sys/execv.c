#include <assert.h>
#include <errno.h>
#include <manos.h>
#include <torgo/commands.h>
#include <string.h>

#include <manos/list.h>
#include <arch/k70/derivative.h>

extern __sysopen(Proc*, const char*, Caps);

static void __manos_exit(void) {
#ifdef PLATFORM_K70CW
    rp->state = ProcDead;
    YIELD();
#endif
}

static void setupStack(Proc* p, Cmd cmd, int argc, char * const argv[]) {
    uint32_t* sp = p->stack + MANOS_ARCH_K70_STACK_SIZE;

    *(--sp) = 0x1000000;              /* XPSR */
    *(--sp) = (uint32_t)cmd;          /* PC   */
    *(--sp) = (uint32_t)__manos_exit; /* LR   */
    *(--sp) = 0x0c0c0c0c;             /* r12  */
    *(--sp) = 0x03030303;             /* r3   */
    *(--sp) = 0x02020202;             /* r2   */
    *(--sp) = (uint32_t)argv;         /* r1   */
    *(--sp) = (uint32_t)argc;         /* r0   */
    *(--sp) = 0xfffffff9;             /* interrupt LR */
    *(--sp) = 0x0b0b0b0b;             /* r11  */
    *(--sp) = 0x0a0a0a0a;             /* r10  */
    *(--sp) = 0x09090909;             /* r9   */
    *(--sp) = 0x08080808;             /* r8   */
    *(--sp) = 0x07070707;             /* r7   */
    *(--sp) = 0x06060606;             /* r6   */
    *(--sp) = 0x05050505;             /* r5   */
    *(--sp) = 0x04040404;             /* r4   */
    *(--sp) = 0x00000000;             /* SVCALLACT bit 0 */

    p->sp = (uintptr_t)sp;
}

Proc* schedProc(Cmd cmd, int argc, char * const argv[]) {
    Proc* p = newProc();

    p->slash = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
    p->dot   = deviceTable[fromDeviceId(DEV_DEVROOT)]->attach("");
#ifdef PLATFORM_K70CW
    p->tty   = __sysopen(p, "/dev/uart/k70Uart", CAP_READWRITE);
#else
    p->tty   = __sysopen(p, "/dev/uart/stdio", CAP_READWRITE);
#endif

    setupStack(p, cmd, argc, argv);
    DISABLE_INTERRUPTS();
    listAddBefore(&p->nextRunQ, &procRunQ);
    p->state = ProcReady;
    return p;
}

/*
 * This is a hack syscall which 'executes' a file.
 * These files must be mode 0555 and have their contents
 * begin with #!<x> where <x> will map to a builtin
 *
 * TOTAL HACK!
 */
int sysexecv(const char *path, char * const argv[]) {
    int argc;
    int ret = -1;
    char *buf = NULL;
    Portal* p = NULL;
    Path* pth = NULL;

    if (!path) {
        errno = EINVAL;
        goto error;
    }

    int isRel = *path != '/';
    /*
     * While various standards exist for maximum args (ARG_MAX, _SC_ARG_MAX)
     * Since our runtime is stll being defined for now we just punt at 256
     */
    for (argc = 0; argc < 256; argc++) {
        if (argv[argc] == NULL) /* this function mandates that argv[argc] == NULL */
            break;
    }

    errno = EPERM;

    if (!(pth = mkPath(path)))
        goto error;

    /* Simple command searching. If the command isn't found on the first walk:
     * If the path is relative, try again from /bin
     */
    if (((p = syswalk(isRel?rp->dot:rp->slash, pth->elems, pth->nelems)) == NULL) && isRel) {
        char *bin = "bin";
        Portal* pbin = syswalk(rp->slash, &bin, 1);
        if (!pbin) {
            errno = EIO;
            goto error;
        }
        p = syswalk(pbin, pth->elems + pth->nelems - 1, 1);
        syskfree(pbin);
    }
    if (p && (p->crumb.flags & CRUMB_ISFILE)) {
        NodeInfo ni;
        if (deviceTable[p->device]->getInfo(p, &ni) == -1)
            goto error;

        if (ni.mode != 0555) {
            errno = EPERM;
            goto error;
        }

        if (deviceTable[p->device]->open(p, CAP_READ) == NULL)
            goto error;

        buf = syskmalloc(ni.length + 1);
        if (deviceTable[p->device]->read(p, buf, ni.length, 0) == -1)
            return -1;

        deviceTable[p->device]->close(p);

        char* c = buf;
        if (c[0] != '#' || c[1] != '!') {
            errno = EPERM;
            goto error;
        }

        c += 2;
        for (unsigned i = 0; i < COUNT_OF(builtinCmds); i++) {
            if (strcmp(builtinCmds[i].cmdName, c) == 0) {
                schedProc(builtinCmds[i].cmd, argc, argv);
                ret = 0;
                ENABLE_INTERRUPTS();
                break;
            }
        }
    }

error:
    if (pth) syskfree(pth);
    if (p)   syskfree(p);
    if (buf) syskfree(buf);
    return ret;
}
