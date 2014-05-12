#include <manos.h>

/**
 * downHeapQ() - maintain the heap invariant after removing a heap item
 * @q:           the heap
 * @lowIndex:    controls the recursive depth of the heaping
 *
 * Return:
 *  @q
 */
HeapQ* downHeapQ(HeapQ* q, size_t startIndex, size_t bottomIndex) {
    size_t maxIndex;
    size_t leftIndex;
    size_t rightIndex;

    leftIndex = startIndex * 2 + 1;
    rightIndex = startIndex * 2 + 2;

    if (leftIndex <= bottomIndex) {
        if (leftIndex == bottomIndex) 
            maxIndex = leftIndex;
        else
            maxIndex = (q->buf[leftIndex] <= q->buf[rightIndex]) ? rightIndex : leftIndex;

        if (q->buf[startIndex] <= q->buf[maxIndex]) {
            uint32_t tmp = q->buf[startIndex];
            q->buf[startIndex] = q->buf[maxIndex];
            q->buf[maxIndex] = tmp;
            q = downHeapQ(q, maxIndex, bottomIndex);
        }
    }

    return q;
}

HeapQ* upHeapQ(HeapQ* q, size_t startIndex, size_t bottomIndex) {
    size_t upIndex;

    if (bottomIndex > startIndex) {
        upIndex = (bottomIndex - 1) / 2;
        if (q->buf[upIndex] < q->buf[bottomIndex]) {
            uint32_t tmp = q->buf[upIndex];
            q->buf[upIndex] = q->buf[bottomIndex];
            q->buf[bottomIndex] = tmp;
            q = upHeapQ(q, startIndex, upIndex);
        }
    }

    return q;
}

/**
 * enqueueHeapQ() - enqueue value onto heap
 * @q:              heap
 * @x:              value
 *
 * Return:
 *   1 - OK
 *   0 - No room
 */
int pushHeapQ(HeapQ* q, uint32_t x) {
    int ok;
    if ((ok = (q->n < q->size))) {
        q->buf[q->n] = x;
        upHeapQ(q, 0, q->n);
        q->n++;
    }

    return ok;
}

/**
 * dequeueHeapQ() - dequeue lowest value from heap
 * @q:              heap
 * @x:              pointer to hold value
 *
 * Return:
 *  1 - x is valid
 *  0 - x is invalid, queue empty
 */
int dequeueHeapQ(HeapQ* q, uint32_t* x) {
    *x = q->buf[0];
    q->n--;
    q->buf[0] = q->buf[q->n];
    downHeapQ(q, 0, q->n - 1);
    return (q->n > 0);
}
