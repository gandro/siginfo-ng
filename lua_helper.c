#include <stdlib.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "siginfo-ng.h"

#define LUA_HELPER_STATE "siginfo_Lua_Helper.state"

static int lua_helper_load_plugin(lua_State *L) {
    lua_plugin_load(L, luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    return 0;
}

static int lua_helper_load_plugin_dir(lua_State *L) {
    lua_plugin_load_dir(L, luaL_checkstring(L, 1));
    return 0;
}

static int lua_helper_pushtraceback(lua_State *L) {
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return 1;
    }
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    return 1;
}


void lua_helper_printerror(lua_State *L) {
    log_print(log_Error, "%s\n", lua_tostring(L, -1));
    lua_close(L);
    exit(EXIT_FAILURE);
}

void lua_helper_callfunction(lua_State *L, int nargs, int nresults) {
    /* insert the errorhandler under the function code */
    lua_pushcfunction(L, lua_helper_pushtraceback);
    lua_insert(L, (-2 - nargs));


    /* call function code */
    if(lua_pcall(L, nargs, nresults, (-2 - nargs)) != 0) {
        lua_helper_printerror(L);
    }

    /* remove errorhandler from stack */
    lua_remove(L, (-1 - nresults));
}

lua_State *lua_helper_initstate() {
    lua_State *L = lua_open();

    /* load Lua stdlib */
    luaL_openlibs(L);

    /* create metatable for plugin namespace */
    luaL_newmetatable(L, LUA_PLUGIN_METATABLE);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_rawset(L, -3);

    /* remove metatable from stack */
    lua_pop(L, 1);

    /* push siginfo table to stack */
    lua_createtable(L, 0, 7);

    /* load settings defaults */
    lua_pushstring(L, SIGINFO_SERVER);
    lua_setfield(L, -2, "server");

    lua_pushnumber(L, SIGINFO_PORT);
    lua_setfield(L, -2, "port");

    lua_pushnumber(L, SIGINFO_INTERVAL);
    lua_setfield(L, -2, "interval");

    lua_pushstring(L, SIGINFO_COMPUTER);
    lua_setfield(L, -2, "computer");

    lua_pushnumber(L, 0);
    lua_setfield(L, -2, "uptime");

    lua_pushstring(L, CLIENT_NAME " v" CLIENT_VERSION);
    lua_setfield(L, -2, "version");

    lua_createtable(L, 0, SIGINFO_ROWS);
    lua_setfield(L, -2, "layout");

    /* and set global */
    lua_setglobal(L, "siginfo");

    /* register wrapper functions */
    lua_register(L, "load_plugin", lua_helper_load_plugin);
    lua_register(L, "load_plugin_dir", lua_helper_load_plugin_dir);

    return L;
}
