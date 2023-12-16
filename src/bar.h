#ifndef BAR_H
#define BAR_H

#include <stddef.h>

#include "block.h"

#define MAX_BLOCKS 5

typedef struct {
    block_t blocks[MAX_BLOCKS];
    size_t num_blocks;
} bar_t;

void bar_init(bar_t *bar);
bool bar_add(bar_t *bar, const struct block *block);
bool bar_add_privdata(bar_t *bar, const struct block *block, void *privdata);
bool bar_init_blocks(bar_t *bar);
void bar_update(bar_t *bar);
void bar_wait(bar_t *bar);

#endif
