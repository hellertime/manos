#include <errno.h>
#include <manos.h>

/**
 * newFifoQ() - allocate memory for a queue
 * @size        queue depth
 *
 * Return:
 *   pointer suitable for freeing with kfree
 */
FifoQ* newFifoQ(size_t size) {
    FifoQ* q = kmalloc((sizeof *q) + size);
    if (!q) {
        errno = ENOMEM;
        return NULL;
    }

    q->size = size;
    return clearFifoQ(q);
}
