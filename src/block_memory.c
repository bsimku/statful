#include "blocks.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#define PROC_MEMINFO_FN "/proc/meminfo"
#define GBYTE 1048576

static bool block_memory_update(void *unused) {
    FILE *f_meminfo = fopen(PROC_MEMINFO_FN, "r");

    if (f_meminfo == NULL) {
        fprintf(stderr, "failed to open '%s': %s", PROC_MEMINFO_FN, strerror(errno));
        return false;
    }

    char ident[32];
    unsigned int kbytes;

    unsigned int mem_total = 0, mem_free = 0;

    while (fscanf(f_meminfo, "%31[^:]: %u kB\n", ident, &kbytes) != EOF) {
        if (strcmp(ident, "MemTotal") == 0) {
            mem_total = kbytes;
        }
        else if (strcmp(ident, "MemFree") == 0) {
            mem_free = kbytes;
        }

        if (mem_total && mem_free)
            break;
    }

    fclose(f_meminfo);

    const float used = (float)(mem_total - mem_free) / GBYTE;
    const float total = (float)mem_total / GBYTE;

    printf("%.1f/%.1f GB", used, total);

    return true;
}

const struct block block_memory = {
    .name = "memory",
    .probe = NULL,
    .init = NULL,
    .update = block_memory_update
};
