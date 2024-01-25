#include "blocks.h"

#include <stdlib.h>
#include <string.h>

#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

#include "block_lua.h"

static int get_lua_func_idx(struct block_lua_privdata *priv, const char *func_name) {
    lua_getfield(priv->lua, -1, func_name);

    if (!lua_isfunction(priv->lua, -1)) {
        fprintf(stderr, "%s: couldn't find %s function.\n", priv->file_path, func_name);
        return -1;
    }

    return luaL_ref(priv->lua, LUA_REGISTRYINDEX);
}

static bool block_lua_init(void **ptr) {
    if (!ptr)
        return false;

    struct block_lua_privdata *priv = *ptr;

    if (!priv)
        return false;

    if (!priv->fn_init_idx) {
        if (luaL_dofile(priv->lua, priv->file_path) != LUA_OK) {
            fprintf(stderr, "%s\n", lua_tostring(priv->lua, -1));
            return false;
        }

        char block[strlen("block_") + strlen(priv->block_name) + 1];

        snprintf(block, sizeof(block), "block_%s", priv->block_name);

        lua_getglobal(priv->lua, block);

        if (!lua_istable(priv->lua, -1)) {
            fprintf(stderr, "%s: couldn't find %s.\n", priv->file_path, block);
            return false;
        }

        if ((priv->fn_init_idx = get_lua_func_idx(priv, "init")) == -1)
            return false;

        if ((priv->fn_update_idx = get_lua_func_idx(priv, "update")) == -1)
            return false;

        if ((priv->fn_get_var_idx = get_lua_func_idx(priv, "get_var")) == -1)
            return false;

        if ((priv->fn_close_idx = get_lua_func_idx(priv, "close")) == -1)
            return false;

        if ((priv->table_idx = luaL_ref(priv->lua, LUA_REGISTRYINDEX)) == -1)
            return false;
    }

    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->fn_init_idx);
    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->table_idx);
    lua_pcall(priv->lua, 1, 1, 0);

    if (!lua_toboolean(priv->lua, -1))
        return false;

    return true;
}

static bool block_lua_update(void *ptr) {
    struct block_lua_privdata *priv = ptr;

    if (!priv)
        return false;

    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->fn_update_idx);
    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->table_idx);
    lua_pcall(priv->lua, 1, 1, 0);

    if (!lua_toboolean(priv->lua, -1))
        return false;

    return true;
}

static char *block_lua_get_var(void *ptr, const char *name, const char *fmt, char *output, size_t size) {
    struct block_lua_privdata *priv = ptr;

    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->fn_get_var_idx);
    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->table_idx);
    lua_pushstring(priv->lua, name);
    lua_pcall(priv->lua, 2, 1, 0);

    if (lua_isnumber(priv->lua, -1)) {
        const int sz = snprintf(output, size, fmt, lua_tonumber(priv->lua, -1));

        if (sz < size) {
            output += sz;
        }
    }
    else if (lua_isstring(priv->lua, -1)) {
        const int sz = snprintf(output, size, fmt, lua_tostring(priv->lua, -1));

        if (sz < size) {
            output += sz;
        }
    }

    return output;
}

static bool block_lua_close(void *ptr) {
    struct block_lua_privdata *priv = ptr;

    if (!priv)
        return false;

    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->fn_close_idx);
    lua_rawgeti(priv->lua, LUA_REGISTRYINDEX, priv->table_idx);
    lua_pcall(priv->lua, 1, 1, 0);

    free(priv->file_path);
    free(priv->block_name);

    lua_State *lua = priv->lua;

    free(priv);

    if (!lua_toboolean(lua, -1))
        return false;

    return true;
}

const struct block block_lua = {
    .name = "lua",
    .probe = NULL,
    .init = block_lua_init,
    .update = block_lua_update,
    .get_var = block_lua_get_var,
    .close = block_lua_close
};
