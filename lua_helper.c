#include <stdlib.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "siginfo-ng.h"

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

static int lua_helper_composite_index(lua_State *L) {
    /* copy child key */
    lua_pushvalue(L, -1);

    /* get child from table */
    lua_rawget(L, -3);
    if(lua_isnil(L, -1)) {
        lua_pushvalue(L, -2);
        lua_newtable(L);

        luaL_getmetatable(L, LUA_PLUGIN_COMPOSITE);
        lua_setmetatable(L, -2);

        lua_settable(L, -5);
    }

    lua_pop(L, 1);
    lua_rawget(L, -2);

    return 1;
}

static int lua_helper_globalacc_index(lua_State *L) {
    /* check global */
    lua_pushvalue(L, 2);
    lua_rawget(L, LUA_GLOBALSINDEX);
    if(!lua_isnil(L, -1)) {
        return 1;
    } else {
        lua_pop(L, 1);
    }

    /* check siginfo.ng.plugins */
    lua_helper_getfield_siginfo_ng(L, "plugins");
    lua_getfield(L, -1, luaL_checkstring(L, 2));
    if(!lua_isnil(L, -1)) {
        return 1;
    } else {
        lua_pop(L, 2);
    }

    /* check siginfo.ng.template */
    lua_helper_getfield_siginfo_ng(L, "template");
    lua_pushvalue(L, 2);
    lua_rawget(L, -2);
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

void lua_helper_getfield_siginfo_ng(lua_State *L, const char *field) {
    lua_getglobal(L, "siginfo");
    lua_getfield(L, -1, "ng");
    lua_getfield(L, -1, field);
    lua_remove(L, -3);
    lua_remove(L, -2);
}

void lua_helper_setfield_siginfo_ng(lua_State *L, const char *field) {
    lua_getglobal(L, "siginfo");
    lua_getfield(L, -1, "ng");
    lua_remove(L, -2);
    lua_insert(L, -2);
    lua_setfield(L, -2, field);
    lua_pop(L, 1);
}

static void lua_helper_register_metatables(lua_State *L) {
    /* create metatable for global access */
    luaL_newmetatable(L, LUA_PLUGIN_GLOBALACC);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_helper_globalacc_index);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    /* create metatable for plugin namespace */
    luaL_newmetatable(L, LUA_PLUGIN_NAMESPACE);
    lua_pushstring(L, "__index");
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_rawset(L, -3);
    lua_pop(L, 1);

    /* create metatable for template variables rootnode */
    luaL_newmetatable(L, LUA_PLUGIN_COMPOSITE);
    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_helper_composite_index);
    lua_rawset(L, -3);
    lua_pop(L, 1);
}

lua_State *lua_helper_initstate() {
    lua_State *L = lua_open();

    /* load Lua stdlib */
    luaL_openlibs(L);
    lua_helper_register_metatables(L);

    /* push siginfo table to stack */
    lua_createtable(L, 0, 8);
    lua_newtable(L);
    lua_setfield(L, -2, "ng");
    lua_setglobal(L, "siginfo");

    /* create template variables rootnode */
    lua_newtable(L);
    luaL_getmetatable(L, LUA_PLUGIN_COMPOSITE);
    lua_setmetatable(L, -2);
    lua_helper_setfield_siginfo_ng(L, "template");

    /* create plugin namespace table */
    lua_newtable(L);
    lua_helper_setfield_siginfo_ng(L, "plugins");

    lua_settings_load_defaults(L);
    lua_plugin_register_api(L);

    luaL_getmetatable(L, LUA_PLUGIN_GLOBALACC);
    lua_setmetatable(L, LUA_GLOBALSINDEX);

    return L;
}
