#include <manos.h>

/**
 * clearHeapQ() - reset the heap and discard contents
 * @q:            heap q to reset
 *
 * Return:
 *  @q
 */
HeapQ* clearHeapQ(HeapQ* q) {
    size_t size = q->size;
    kmemset(q, 0, sizeof *q);
    q->size = size;
    return q;
}
