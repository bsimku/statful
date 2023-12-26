#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"
#include "block_common.h"

#define PROC_STAT_FN "/proc/stat"

struct block_cpu_data {
    FILE *f_stat;
    unsigned long active;
    unsigned long total;
    int cpu_used;
};

static bool block_cpu_init(void **opaque) {
    if (opaque == NULL)
        return false;

    struct block_cpu_data *data = alloc_zero(sizeof(struct block_cpu_data));

    data->f_stat = fopen(PROC_STAT_FN, "r");

    if (data->f_stat == NULL) {
        fprintf(stderr, "failed to open '%s': %s", PROC_STAT_FN, strerror(errno));
        return false;
    }

    *opaque = data;

    return true;
}

static bool block_cpu_update(void *opaque) {
    struct block_cpu_data *data = opaque;

    if (data == NULL)
        return false;

    data->cpu_used = -1;

    return true;
}

static int get_cpu_used(struct block_cpu_data *data) {
    char cpu[4];
    unsigned int user, nice, system, idle, iowait, irq, softirq, steal, guest, guest_nice;

    rewind(data->f_stat);

    fscanf(data->f_stat, "%3s %u %u %u %u %u %u %u %u %u %u", cpu, &user, &nice, &system,
           &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);

    fflush(data->f_stat);

    const unsigned long active = user + system + nice + softirq + steal;
    const unsigned long total = user + system + nice + softirq + steal + idle + iowait;

    const unsigned long active_diff = active - data->active;
    const unsigned long total_diff = total - data->total;

    data->active = active;
    data->total = total;

    if (total_diff == 0)
        return false;

    return active_diff * 100 / total_diff;
}

static int get_var_cpu_used(struct block_cpu_data *data) {
    if (data->cpu_used == -1) {
        data->cpu_used = get_cpu_used(data);
    }

    return data->cpu_used;
}

static char *block_cpu_get_var(void *ptr, const char *name, const char *fmt, char *output, size_t size) {
    struct block_cpu_data *data = ptr;

    if (data == NULL)
        return NULL;

    BLOCK_PARAM("cpu_used", fmt, get_var_cpu_used(data));

    return output;
}

static bool block_cpu_close(void *opaque) {
    struct block_cpu_data *data = opaque;

    if (data == NULL)
        return false;

    if (fclose(data->f_stat) == EOF) {
        fprintf(stderr, "fclose() failed: %s\n", strerror(errno));
        return false;
    }

    free(data);

    return true;
}

const struct block block_cpu = {
    .name = "cpu",
    .probe = NULL,
    .init = block_cpu_init,
    .update = block_cpu_update,
    .get_var = block_cpu_get_var,
    .close = block_cpu_close
};
