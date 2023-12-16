#include "blocks.h"

#include <string.h>

const struct block *blocks_find_native(const char *name) {
    for (int i = 0; i < sizeof(native_blocks) / sizeof(native_blocks[0]); i++) {
        if (strcmp(native_blocks[i]->name, name) == 0)
            return native_blocks[i];
    }

    return NULL;
}
