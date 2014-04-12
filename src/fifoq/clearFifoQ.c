#include <manos.h>
#include <string.h>

/**
 * clearFifoQ() - reset the queue, discard contents
 * @q             queue to reset
 *
 * Return:
 *   @q
 */
FifoQ* clearFifoQ(FifoQ* q) {
    memset(q, 0, sizeof *q);
    q->isEmpty = 1;
    return q;
}
