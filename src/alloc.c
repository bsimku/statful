#include "alloc.h"

#include "stdlib.h"

void *alloc(size_t size) {
    void *ptr = malloc(size);

    if (ptr == NULL) {
        abort();
    }

    return ptr;
}

void *alloc_zero(size_t size) {
    void *ptr = calloc(1, size);

    if (ptr == NULL) {
        abort();
    }

    return ptr;
}
