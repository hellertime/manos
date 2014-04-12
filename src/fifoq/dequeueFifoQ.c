#include <manos.h>

/**
 * dequeueFifoQ() - dequeue if data is available
 * @q:              fifo q to dequeue from
 * @c:              pointer to hold output
 *
 * Return:
 *   1 - dequeued byte
 *   0 - nothing to dequeue
 */
int dequeueFifoQ(FifoQ* q, char* c) {
    if (!q || q->isEmpty)
        return 0;

    *c = q->buf[q->readOffset++];
    q->readOffset %= q->size;
    q->isEmpty = (q->readOffset == q->writeOffset);
    return 1;
}
