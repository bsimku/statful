#ifndef BLOCKS_H
#define BLOCKS_H

#include "block.h"

extern const struct block block_lua;

extern const struct block block_battery;
extern const struct block block_cpu;
extern const struct block block_memory;
extern const struct block block_time;
extern const struct block block_volume;

const static struct block *native_blocks[] = {
    &block_battery,
    &block_cpu,
    &block_memory,
    &block_time,
    &block_volume
};

const struct block *blocks_find_native(const char *name);

#endif
