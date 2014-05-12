#include <errno.h>
#include <manos.h>
#include <stddef.h>

/**
 * newHeapQ() - allocate memory for a priority heap
 * @size:       heap depth
 * Return:
 *   pointer suitable for freeing with kfree
 */
HeapQ* newHeapQ(size_t size) {
    HeapQ* q = syskmalloc(sizeof(*q) + (sizeof(intptr_t) * size));
    if (!q) {
        errno = ENOMEM;
        return NULL;
    }

    q->size = size;
    return clearHeapQ(q);
}
