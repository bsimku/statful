#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alloc.h"
#include "block_common.h"

struct block_time_data {
    time_t secs;
    struct tm *curr_time;
    char date[11];
    char time[9];
};

static bool block_time_init(void **ptr) {
    if (ptr == NULL)
        return false;

    *ptr = alloc(sizeof(struct block_time_data));

    return true;
}

static bool block_time_update(void *ptr) {
    struct block_time_data *data = ptr;

    if (data == NULL)
        return NULL;

    data->secs = time(NULL);

    if (data->secs == (time_t) - 1) {
        fprintf(stderr, "time() failed: %s", strerror(errno));
        return false;
    }

    data->curr_time = localtime(&data->secs);

    if (data->curr_time == NULL) {
        fprintf(stderr, "localtime() failed: %s", strerror(errno));
        return false;
    }

    data->date[0] = '\0';
    data->time[0] = '\0';

    return true;
}

static char *get_var_date(struct block_time_data *data) {
    if (data->date[0] == '\0') {
        snprintf(data->date, sizeof(data->date) / sizeof(data->date[0]), "%d-%02d-%02d", data->curr_time->tm_year + 1900, data->curr_time->tm_mon + 1, data->curr_time->tm_mday);
    }

    return data->date;
}

static char *get_var_time(struct block_time_data *data) {
    if (data->time[0] == '\0') {
        snprintf(data->time, sizeof(data->time) / sizeof(data->time[0]), "%02d:%02d:%02d", data->curr_time->tm_hour, data->curr_time->tm_min, data->curr_time->tm_sec);
    }

    return data->time;
}

static char *block_time_get_var(void *ptr, const char *name, const char *fmt, char *output, size_t size) {
    struct block_time_data *data = ptr;

    if (data == NULL)
        return NULL;

    BLOCK_PARAM("date", fmt, get_var_date(data));
    BLOCK_PARAM("time", fmt, get_var_time(data));

    return output;
}

static bool block_time_close(void *ptr) {
    struct block_time_data *data = ptr;

    if (data == NULL)
        return false;

    free(data);

    return true;
}

const struct block block_time = {
    .name = "time",
    .probe = NULL,
    .init = block_time_init,
    .update = block_time_update,
    .get_var = block_time_get_var,
    .close = block_time_close
};
