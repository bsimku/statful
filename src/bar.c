#include "bar.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "block_time.h"

static bool update_block(block_t *block) {
    return block->funcs.update(block->opaque);
}

static bool init_block(block_t *block) {
    if (!block->funcs.init)
        return true;

    if (!block->funcs.init(&block->opaque)) {
        fprintf(stderr, "failed to initialize block .... \n");
        return false;
    }

    return true;
}

void bar_init(bar_t *bar) {
    bar->num_blocks = 0;
}

// TODO: make struct block a pointer
void bar_add(bar_t *bar, const struct block block) {
    if (block.probe && !block.probe())
        return;

    if (bar->num_blocks >= MAX_BLOCKS) {// TODO: error out
        fprintf(stderr, "max block number exceeded.\n");
        return;
    }

    block_t *bar_block = &bar->blocks[bar->num_blocks++];

    bar_block->funcs = block;

    if (!init_block(bar_block)) {
        exit(1);
    }
}

void bar_update(bar_t *bar) {
    for (size_t i = 0; i < bar->num_blocks; i++) {
        if (!update_block(&bar->blocks[i]))
            continue;

        if (i + 1 < bar->num_blocks) {
            printf(" | ");
        }
    }

    printf("\n");
}

void bar_wait(bar_t *bar) {
    sleep(1);
}
