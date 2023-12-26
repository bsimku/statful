#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>

struct block {
    const char *name;
    bool (*probe)();
    bool (*init)(void **);
    bool (*close)(void *);
    bool (*update)(void *);
};

typedef struct {
    const struct block *funcs;
    char *format;
    void *opaque;
} block_t;

#endif
