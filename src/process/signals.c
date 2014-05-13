#include <manos.h>

/**
 * setSignalMask() - replace oldMask
 *
 * @mask:            new mask
 * @oldMask:         holds old mask value (if not NULL)
 *
 * Return:
 *  0 - OK
 */
int setSignalMask(uint32_t mask, uint32_t* oldMask) {
    if (oldMask)
        *oldMask = rp->sigMask;

    rp->sigMask = mask;
    return 0;
}

/**
 * setSignalBlock() - union mask with current mask
 * @mask:             additional mask
 * @oldMask:          previous mask
 *
 * Return:
 *  0 - OK
 */
int setSignalBlock(uint32_t mask, uint32_t* oldMask) {
    if (oldMask)
        *oldMask = rp->sigMask;

    rp->sigMask |= mask;
    return 0;
}

/**
 * setSignalUnblock() - toggle off values in old mask
 */
int setSignalUnblock(uint32_t mask, uint32_t* oldMask) {
    if (oldMask)
        *oldMask = rp->sigMask;

    rp->sigMask |= rp->sigMask & ~mask;
    return 0;
}
