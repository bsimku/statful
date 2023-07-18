#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef enum {
    BLOCK_TIME,
    MAX_BLOCKS
} block_type_e;

typedef struct {
    unsigned num_blocks;
    block_type_e blocks[MAX_BLOCKS];
} config_t;

bool config_read(const char *filename, config_t *config);

#endif
