#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stddef.h>

struct block {
    const char *name;
    bool (*probe)();
    bool (*init)(void **);
    bool (*close)(void *);
    bool (*update)(void *);
    char *(*get_var)(void *, const char *, const char *, char *, size_t);
};

typedef struct {
    const struct block *funcs;
    char *format;
    void *opaque;
} block_t;

#endif
