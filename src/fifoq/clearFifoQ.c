#include <manos.h>

/**
 * clearFifoQ() - reset the queue, discard contents
 * @q             queue to reset
 *
 * Return:
 *   @q
 */
FifoQ* clearFifoQ(FifoQ* q) {
    size_t size = q->size;
    kmemset(q, 0, sizeof *q);
    q->size = size;
    q->isEmpty = 1;
    return q;
}
