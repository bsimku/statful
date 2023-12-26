#ifndef BLOCK_LUA_H
#define BLOCK_LUA_H

#include <luajit.h>

struct block_lua_privdata {
    lua_State *lua;
    char *file_path;
    char *block_name;
    int table_idx;
    int fn_init_idx;
    int fn_update_idx;
    int fn_get_var_idx;
    int fn_close_idx;
};

#endif
