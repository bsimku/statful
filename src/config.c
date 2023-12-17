#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define CONFIG_DIR_NAME "/statful"
#define CONFIG_BLOCKS_SUBDIR "/blocks"
#define CONFIG_FILE_NAME "/config.lua"

static char *get_config_dir() {
    const char *xdg_home = getenv("XDG_CONFIG_HOME");

    if (xdg_home && xdg_home[0] != '\0') {
        const size_t size = strlen(xdg_home) + strlen(CONFIG_DIR_NAME) + 1;

        char *path = calloc(size, sizeof(char));

        if (!path)
            return NULL;

        snprintf(path, size, "%s%s", xdg_home, CONFIG_DIR_NAME);

        return path;
    }

    const char *home_path = getenv("HOME");

    if (home_path) {
        const size_t size = strlen(home_path) + strlen("/.config") + strlen(CONFIG_DIR_NAME) + 1;

        char *path = calloc(size, sizeof(char));

        if (!path)
            return NULL;

        snprintf(path, size, "%s/.config%s", home_path, CONFIG_DIR_NAME);

        return path;
    }

    return NULL;
}

char *config_get_path() {
    char *path;

    char *config_dir = get_config_dir();

    if (!config_dir)
        return NULL;

    const size_t size = strlen(config_dir) + strlen(CONFIG_FILE_NAME) + 1;

    path = calloc(size, sizeof(char));

    if (!path)
        goto cleanup;

    snprintf(path, size, "%s%s", config_dir, CONFIG_FILE_NAME);

cleanup:
    free(config_dir);

    return path;
}

char *config_get_block_path(const char *name) {
    char *path;

    char *config_dir = get_config_dir();

    if (!config_dir)
        return NULL;

    const size_t size = strlen(config_dir) + strlen(CONFIG_BLOCKS_SUBDIR) + 1 + strlen(name) + strlen(".lua") + 1;

    path = calloc(size, sizeof(char));

    if (!path)
        goto cleanup;

    snprintf(path, size, "%s%s/%s.lua", config_dir, CONFIG_BLOCKS_SUBDIR, name);

cleanup:
    free(config_dir);

    return path;
}

