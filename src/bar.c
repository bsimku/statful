#include "bar.h"

#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define UPDATE_INTERVAL_USECS 1000000
#define JITTER_TIME_USECS     50000
#define NSECS_IN_USEC         1000

static bool reinit_block(block_t *block) {
    if (block->funcs->close && !block->funcs->close(block->opaque))
        return false;

    if (block->funcs->probe && !block->funcs->probe())
        return false;

    if (!block->funcs->init)
        return true;

    return block->funcs->init(&block->opaque);
}

void bar_init(bar_t *bar) {
    bar->num_blocks = 0;
}

bool bar_add(bar_t *bar, const struct block *block) {
    return bar_add_privdata(bar, block, NULL);
}

bool bar_add_privdata(bar_t *bar, const struct block *block, void *privdata) {
    if (block->probe && !block->probe())
        return false;

    if (bar->num_blocks >= MAX_BLOCKS) {
        fprintf(stderr, "max block number exceeded.\n");
        return false;
    }

    block_t *bar_block = &bar->blocks[bar->num_blocks++];

    bar_block->funcs = block;
    bar_block->opaque = privdata;

    return true;
}

bool bar_init_blocks(bar_t *bar) {
    for (size_t i = 0; i < bar->num_blocks; i++) {
        block_t *block = &bar->blocks[i];

        if (!block->funcs->init)
            continue;

        if (!block->funcs->init(&block->opaque)) {
            fprintf(stderr, "failed to initialize block %s\n", block->funcs->name);
            return false;
        }
    }

    return true;
}

void bar_update(bar_t *bar) {
    for (size_t i = 0; i < bar->num_blocks; i++) {
        block_t *block = &bar->blocks[i];

        if (!block->funcs->update(block->opaque)) {
            if (!reinit_block(block)) {
                fprintf(stderr, "failed to reinit block %s\n", block->funcs->name);
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

void bar_close(bar_t *bar) {
    for (size_t i = 0; i < bar->num_blocks; i++) {
        block_t *block = &bar->blocks[i];

        if (block->funcs->close) {
            block->funcs->close(block->opaque);
        }
    }
}
