#ifndef BAR_H
#define BAR_H

#include <stddef.h>

#include "block.h"

typedef struct {
    block_t *blocks;
    size_t blocks_size;
    size_t num_blocks;
} bar_t;

bool bar_init(bar_t *bar, size_t max_blocks);
bool bar_add(bar_t *bar, const struct block *block, const char *fmt);
bool bar_add_privdata(bar_t *bar, const struct block *block, const char *fmt, void *privdata);
bool bar_init_blocks(bar_t *bar);
void bar_update(bar_t *bar);
void bar_wait(bar_t *bar);
void bar_close(bar_t *bar);

#endif
