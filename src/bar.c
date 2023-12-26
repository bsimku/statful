#include "bar.h"

#include <bits/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define UPDATE_INTERVAL_USECS 1000000
#define JITTER_TIME_USECS     50000
#define NSECS_IN_USEC         1000

static bool reinit_block(block_t *block) {
    if (block->funcs->close && !block->funcs->close(block->opaque))
        return false;

    block->opaque = NULL;

    if (block->funcs->probe && !block->funcs->probe())
        return false;

    if (!block->funcs->init)
        return true;

    return block->funcs->init(&block->opaque);
}

bool bar_init(bar_t *bar, size_t max_blocks) {
    bar->blocks_size = max_blocks;
    bar->blocks = calloc(sizeof(block_t), max_blocks);

    if (!bar->blocks)
        return false;

    bar->num_blocks = 0;

    return true;
}

bool bar_add(bar_t *bar, const struct block *block, const char *fmt) {
    return bar_add_privdata(bar, block, fmt, NULL);
}

bool bar_add_privdata(bar_t *bar, const struct block *block, const char *fmt, void *privdata) {
    if (block->probe && !block->probe())
        return false;

    if (bar->num_blocks >= bar->blocks_size) {
        fprintf(stderr, "maximum block number exceeded.\n");
        return false;
    }

    block_t *bar_block = &bar->blocks[bar->num_blocks++];

    bar_block->funcs = block;
    bar_block->format = strdup(fmt);
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

static char *append(char *str, char *str_end, char c) {
    if (str < str_end) {
        *str++ = c;
    }

    return str;
}

static char *append_str(char *str, char *str_end, char *c) {
    const size_t len = strnlen(c, str_end - str);

    memcpy(str, c, len);
    str += len;

    return str;
}

static char *parse_format(block_t *block, char *out, char *out_end, size_t output_size) {
    enum parse_state {
        none,
        variable,
        format
    } state = none;

    char *var_name;
    char *fmt;

    char *c = block->format;

    while (*c) {
        if (*c == '{') {
            state = variable;
            var_name = c + 1;
        }
        else if (*c == ':' && state == variable) {
            state = format;
            *c = '\0';
            fmt = c + 1;
        }
        else if (*c == '}' && state == format) {
            state = none;
            *c = '\0';

            out = block->funcs->get_var(block->opaque, var_name, fmt, out, output_size);

            *c = '}';
            *(fmt - 1) = ':';
        }
        else if (state == none) {
            out = append(out, out_end, *c);
        }

        c++;
    }

    return out;
}

void bar_update(bar_t *bar) {
    const size_t output_size = 256;
    char output[output_size];

    char *out = output;
    char *out_end = output + output_size;

    for (size_t i = 0; i < bar->num_blocks; i++) {
        block_t *block = &bar->blocks[i];

        if (!block->funcs->update(block->opaque)) {
            if (!reinit_block(block)) {
                fprintf(stderr, "failed to reinit block %s\n", block->funcs->name);
            }

            continue;
        }

        out = parse_format(block, out, out_end, output_size);

        if (i + 1 < bar->num_blocks) {
            out = append_str(out, out_end, " | ");
        }
    }

    append(out, out_end, '\n');
    append(out, out_end, '\0');

    puts(output);
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

    free(bar->blocks);
}
