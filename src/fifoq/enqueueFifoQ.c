#include <manos.h>

/**
 * isFullFifoQ() - test for fullness
 * @q:             queue to test
 *
 * Return:
 *   bool yes no
 */
int isFullFifoQ(FifoQ* q) {
    return (q && (!q->isEmpty) && (q->readOffset == q->writeOffset));
}

/**
 * enqueueFifoQ() - enqueue if possible
 * @q:              fifo queue to enqueue on
 * @c:              char to enqueue
 *
 * Return:
 *   1 - enqueued byte
 *   0 - nothing enqueued
 */
int enqueueFifoQ(FifoQ* q, char c) {
    if (isFullFifoQ(q))
        return 0;

    q->buf[q->writeOffset++] = c;
    q->writeOffset %= q->size;
    q->isEmpty = 0;
    return 1;
}
