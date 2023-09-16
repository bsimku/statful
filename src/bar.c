#include "bar.h"

#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define UPDATE_INTERVAL_USECS 1000000
#define JITTER_TIME_USECS     50000
#define NSECS_IN_USEC         1000

static bool update_block(block_t *block) {
    return block->funcs.update(block->opaque);
}

static bool reinit_block(block_t *block) {
    if (block->funcs.close && !block->funcs.close(block->opaque))
        return false;

    if (block->funcs.probe && !block->funcs.probe())
        return false;

    if (!block->funcs.init)
        return true;

    return block->funcs.init(&block->opaque);
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
        block_t *block = &bar->blocks[i];

        if (!update_block(block)) {
            if (!reinit_block(block)) {
                fprintf(stderr, "failed to reinit block!\n");
            }

            continue;
        }

        if (i + 1 < bar->num_blocks) {
            printf(" | ");
        }
    }

    printf("\n");

    fflush(stdout);
}

void bar_wait(bar_t *bar) {
    struct timespec cur_time;
    clock_gettime(CLOCK_REALTIME, &cur_time);

    usleep(UPDATE_INTERVAL_USECS - cur_time.tv_nsec / NSECS_IN_USEC %
           UPDATE_INTERVAL_USECS + JITTER_TIME_USECS);
}
