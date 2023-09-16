#include "blocks.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "alloc.h"

#define PROC_POWER_SUPPLIES_DIR "/sys/class/power_supply"
#define MAX_FILE_SIZE 32

static char *make_path(const char *dir1, const char *dir2, const char *name) {
    char *path = alloc_zero(strlen(dir1) + strlen(dir2) + strlen(name) + 3);

    strcpy(path, dir1);
    strcat(path, "/");
    strcat(path, dir2);
    strcat(path, "/");
    strcat(path, name);

    return path;
}

static bool read_file(const char *fn, char buf[MAX_FILE_SIZE]) {
    FILE *file = fopen(fn, "r");

    if (file == NULL) {
        fprintf(stderr, "failed to open '%s': %s", fn, strerror(errno));
        return false;
    }

    size_t size = fread(buf, 1, MAX_FILE_SIZE, file);

    fclose(file);

    if (size <= 1) {
        fprintf(stderr, "failed to read '%s': %s", fn, strerror(errno));
        return false;
    }

    buf[size - 1] = '\0';

    return true;
}

static bool is_battery(const char *power_supply) {
    char *type_fn = make_path(PROC_POWER_SUPPLIES_DIR, power_supply, "type");

    char contents[MAX_FILE_SIZE];

    if (!read_file(type_fn, contents)) {
        free(type_fn);
        return false;
    }

    free(type_fn);

    return strcmp(contents, "Battery") == 0;
}

static char *find_battery_path() {
    DIR *dir = opendir(PROC_POWER_SUPPLIES_DIR);

    struct dirent *ent;

    while ((ent = readdir(dir))) {
        if (ent->d_name[0] == '.')
            continue;

        if (!is_battery(ent->d_name))
            continue;

        char *path = alloc(strlen(ent->d_name) + 1);

        strcpy(path, ent->d_name);

        closedir(dir);

        return path;
    }

    closedir(dir);

    return NULL;
}

static bool block_battery_probe() {
    char *path = find_battery_path();

    if (path == NULL)
        return false;

    free(path);

    return true;
}

struct block_battery_data {
    char *status_path;
    char *capacity_path;
};

static bool block_battery_init(void **ptr) {
    if (ptr == NULL)
        return false;

    char *path = find_battery_path();

    if (path == NULL)
        return false;

    struct block_battery_data *data = alloc(sizeof(struct block_battery_data));

    data->status_path = make_path(PROC_POWER_SUPPLIES_DIR, path, "status");
    data->capacity_path = make_path(PROC_POWER_SUPPLIES_DIR, path, "capacity");

    *ptr = data;

    return true;
}

static int get_battery_capacity(const char *capacity_fn) {
    char contents[MAX_FILE_SIZE];

    if (!read_file(capacity_fn, contents))
        return -1;

    return strtol(contents, NULL, 10);
}

enum battery_status {
    UNKNOWN,
    NOT_CHARGING,
    CHARGING,
    DISCHARGING,
    FULL
};

static enum battery_status get_battery_status(const char *status_fn) {
    char contents[MAX_FILE_SIZE];

    if (!read_file(status_fn, contents))
        return -1;

    if (strcmp(contents, "Not charging") == 0)
        return NOT_CHARGING;

    if (strcmp(contents, "Charging") == 0)
        return CHARGING;

    if (strcmp(contents, "Discharging") == 0)
        return DISCHARGING;

    if (strcmp(contents, "Full") == 0)
        return FULL;

    return UNKNOWN;
}

static const char *get_status_symbol(const enum battery_status status, const int capacity) {
    if (status == CHARGING)
        return "";
    else if (capacity < 20)
        return "";
    else if (capacity < 40)
        return "";
    else if (capacity < 60)
        return "";
    else if (capacity < 80)
        return "";

    return "";
}

static bool block_battery_update(void *ptr) {
    const struct block_battery_data *data = (const struct block_battery_data *)ptr;

    if (data == NULL)
        return false;

    const enum battery_status status = get_battery_status(data->status_path);

    if (status == UNKNOWN)
        return false;

    const int capacity = get_battery_capacity(data->capacity_path);

    if (capacity == -1)
        return false;

    printf("%s %u%%", get_status_symbol(status, capacity), capacity);

    return true;
}

const struct block block_battery = {
    .name = "battery",
    .probe = block_battery_probe,
    .init = block_battery_init,
    .update = block_battery_update
};
