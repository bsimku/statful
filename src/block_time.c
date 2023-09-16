#include "blocks.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static bool block_time_update(void *unused) {
    const time_t secs = time(NULL);

    if (secs == (time_t) - 1) {
        fprintf(stderr, "time() failed: %s", strerror(errno));
        return false;
    }

    const struct tm *curr_time = localtime(&secs);

    if (curr_time == NULL) {
        fprintf(stderr, "localtime() failed: %s", strerror(errno));
        return false;
    }

    printf(" %d-%02d-%02d %02d:%02d:%02d", curr_time->tm_year + 1900, curr_time->tm_mon + 1,
                                            curr_time->tm_mday, curr_time->tm_hour,
                                            curr_time->tm_min, curr_time->tm_sec);

    return true;
}

const struct block block_time = {
    .name = "time",
    .probe = NULL,
    .init = NULL,
    .update = block_time_update,
    .close = NULL
};
