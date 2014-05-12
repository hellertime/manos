#include <errno.h>
#include <manos.h>

/**
 * newHeapQ() - allocate memory for a priority heap
 * @size:       heap depth
 * Return:
 *   pointer suitable for freeing with kfree
 */
HeapQ* newHeapQ(size_t size) {
    HeapQ* q = syskmalloc(sizeof(*q) + (sizeof(uint32_t) * size));
    if (!q) {
        errno = ENOMEM;
        return NULL;
    }

    q->size = size;
    return clearHeapQ(q);
}
