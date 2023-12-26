#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "block_common.h"

#define PROC_MEMINFO_FN "/proc/meminfo"
#define GBYTE 1048576

struct block_memory_data {
    float used;
    float total;
};

static bool block_memory_init(void **opaque) {
    if (opaque == NULL)
        return false;

    *opaque = alloc(sizeof(struct block_memory_data));

    return true;
}

static bool block_memory_update(void *opaque) {
    struct block_memory_data *data = opaque;

    if (data == NULL)
        return false;

    data->total = 0;
    data->used = 0;

    return true;
}

bool update_memory_vars(struct block_memory_data *data) {
    FILE *f_meminfo = fopen(PROC_MEMINFO_FN, "r");

    if (f_meminfo == NULL) {
        fprintf(stderr, "failed to open '%s': %s", PROC_MEMINFO_FN, strerror(errno));
        return false;
    }

    char ident[32];
    unsigned int kbytes;

    unsigned int mem_total = 0, mem_available = 0;

    while (fscanf(f_meminfo, "%31[^:]: %u kB\n", ident, &kbytes) != EOF) {
        if (strcmp(ident, "MemTotal") == 0) {
            mem_total = kbytes;
        }
        else if (strcmp(ident, "MemAvailable") == 0) {
            mem_available = kbytes;
        }

        if (mem_total && mem_available)
            break;
    }

    fclose(f_meminfo);

    data->used = (float)(mem_total - mem_available) / GBYTE;
    data->total = (float)mem_total / GBYTE;

    return true;
}

static float get_var_memory_used(struct block_memory_data *data) {
    if (data->used == 0) {
        update_memory_vars(data);
    }

    return data->used;
}

static float get_var_memory_total(struct block_memory_data *data) {
    if (data->total == 0) {
        update_memory_vars(data);
    }

    return data->total;
}

static char *block_memory_get_var(void *ptr, const char *name, const char *fmt, char *output, size_t size) {
    struct block_memory_data *data = ptr;

    if (data == NULL)
        return NULL;

    BLOCK_PARAM("used", fmt, get_var_memory_used(data));
    BLOCK_PARAM("total", fmt, get_var_memory_total(data));

    return output;
}

static bool block_memory_close(void *ptr) {
    struct block_memory_data *data = ptr;

    if (data == NULL)
        return false;

    free(data);

    return true;
}


const struct block block_memory = {
    .name = "memory",
    .probe = NULL,
    .init = block_memory_init,
    .update = block_memory_update,
    .get_var = block_memory_get_var,
    .close = block_memory_close
};
