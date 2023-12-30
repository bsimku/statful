#include "blocks.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "alloc.h"
#include "block_common.h"

#define PROC_POWER_SUPPLIES_DIR "/sys/class/power_supply"
#define MAX_FILE_SIZE 32
#define MICROWATTS_IN_WATT 1000000

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
        fprintf(stderr, "failed to open '%s': %s\n", fn, strerror(errno));
        return false;
    }

    size_t size = fread(buf, 1, MAX_FILE_SIZE, file);

    fclose(file);

    if (size <= 1) {
        fprintf(stderr, "failed to read '%s': %s\n", fn, strerror(errno));
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

enum battery_status {
    UNKNOWN,
    NOT_CHARGING,
    CHARGING,
    DISCHARGING,
    FULL
};

struct block_battery_data {
    char *status_path;
    char *capacity_path;
    char *power_path;
    enum battery_status status;
    int capacity;
    float power;
    const char *symbol;
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
    data->power_path = make_path(PROC_POWER_SUPPLIES_DIR, path, "power_now");

    *ptr = data;

    return true;
}

static int get_battery_capacity(const char *capacity_fn) {
    char contents[MAX_FILE_SIZE];

    if (!read_file(capacity_fn, contents))
        return -1;

    return strtol(contents, NULL, 10);
}

static enum battery_status get_battery_status(const char *status_fn) {
    char contents[MAX_FILE_SIZE];

    if (!read_file(status_fn, contents))
        return UNKNOWN;

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

static float get_battery_power(const char *power_fn) {
    char contents[MAX_FILE_SIZE];

    if (!read_file(power_fn, contents)) {
        return -1;
    }

    return strtof(contents, NULL) / MICROWATTS_IN_WATT;
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
    struct block_battery_data *data = ptr;

    if (data == NULL)
        return false;

    data->status = UNKNOWN;
    data->symbol = NULL;
    data->capacity = -1;
    data->power = -1;

    return true;
}

static int get_var_capacity(struct block_battery_data *data) {
    if (data->capacity == -1) {
        data->capacity = get_battery_capacity(data->capacity_path);
    }

    return data->capacity;
}

static float get_var_power(struct block_battery_data *data) {
    if (data->power == -1) {
        data->power = get_battery_power(data->power_path);
    }

    return data->power;
}

static const char *get_var_symbol(struct block_battery_data *data) {
    if (data->status == UNKNOWN) {
        data->status = get_battery_status(data->status_path);
    }

    if (data->symbol == NULL) {
        data->symbol = get_status_symbol(data->status, get_var_capacity(data));
    }

    return data->symbol;
}

static char *block_battery_get_var(void *ptr, const char *name, const char *fmt, char *output, size_t size) {
    struct block_battery_data *data = ptr;

    if (data == NULL)
        return NULL;

    BLOCK_PARAM("sym", fmt, get_var_symbol(data));
    BLOCK_PARAM("capacity", fmt, get_var_capacity(data));
    BLOCK_PARAM("power", fmt, get_var_power(data));

    return output;
}

static bool block_battery_close(void *ptr) {
    struct block_battery_data *data = ptr;

    if (data == NULL)
        return false;

    if (data->capacity_path)
        free(data->capacity_path);

    if (data->status_path)
        free(data->status_path);

    free(data);

    return true;
}

const struct block block_battery = {
    .name = "battery",
    .probe = block_battery_probe,
    .init = block_battery_init,
    .update = block_battery_update,
    .get_var = block_battery_get_var,
    .close = block_battery_close
};
