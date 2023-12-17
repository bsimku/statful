#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <locale.h>
#include <signal.h>

#include <luajit.h>
#include <lualib.h>
#include <lauxlib.h>

#include "config.h"
#include "bar.h"
#include "blocks.h"
#include "block_lua.h"
#include "lua.h"

int g_signo;

static void trap(int signo) {
    g_signo = signo;
}

static int array_size_lua(lua_State *L) {
    int idx = 1;

    while (true) {
        lua_rawgeti(L, -idx, idx);

        if (lua_isnil(L, -1))
            break;

        idx++;
    }

    lua_settop(L, -idx - 1);

    return idx - 1;
}

static bool apply_config(bar_t *bar, lua_State *L) {
    char *config_path = config_get_path();

    if (!config_path)
        return false;

    if (luaL_dofile(L, config_path) != LUA_OK) {
        fprintf(stderr, "luaL_dofile() failed: %s\n", lua_tostring(L, -1));
        return false;
    }

    lua_getglobal(L, "config");

    if (!lua_istable(L, -1)) {
        fprintf(stderr, "%s: config is not defined\n", config_path);
        return false;
    }

    lua_getfield(L, -1, "blocks");

    if (!lua_istable(L, -1)) {
        fprintf(stderr, "%s: config.blocks is not defined\n", config_path);
        return false;
    }

    const int num_blocks = array_size_lua(L);

    if (!bar_init(bar, num_blocks))
        return false;

    for (int idx = 1; idx <= num_blocks; idx++) {
        lua_rawgeti(L, -idx, idx);

        if (lua_isnil(L, -1))
            break;

        const char *name = lua_tostring(L, -1);

        if (name == NULL || name[0] == '\0')
            continue;

        const struct block *block = blocks_find_native(name);

        if (block) {
            bar_add(bar, block);
        }
        else {
            struct block_lua_privdata *priv = malloc(sizeof(struct block_lua_privdata));

            if (!priv)
                return false;

            priv->lua = L;
            priv->file_path = config_get_block_path(name);
            priv->block_name = strdup(name);
            priv->fn_init_idx = 0;
            priv->fn_update_idx = 0;
            priv->fn_close_idx = 0;

            bar_add_privdata(bar, &block_lua, priv);
        }
    }

    return true;
}

static bool run_bar() {
    lua_State *L = luaL_newstate();

    if (!L)
        return false;

    luaL_openlibs(L);

    bar_t bar;

    if (!apply_config(&bar, L))
        return false;

    bar_init_blocks(&bar);

    while (true) {
        bar_update(&bar);
        bar_wait(&bar);

        if (g_signo == SIGHUP)
            break;
    }

    bar_close(&bar);
    lua_close(L);

    return true;
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");

    const struct sigaction act = {
        .sa_handler = trap
    };

    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGHUP, &act, NULL);

    while (true) {
        g_signo = 0;

        if (!run_bar()) {
            pause();
        }
    }
}
