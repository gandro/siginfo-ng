#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include <lua.h>
#include <lauxlib.h>

#include "siginfo-ng.h"

void lua_plugin_load(lua_State *L, const char *namespace, const char *filename) {
    /* new namespace */
    lua_newtable(L);
    luaL_getmetatable(L, LUA_PLUGIN_NAMESPACE);
    lua_setmetatable(L, -2);

    /* copy namespace to siginfo.ng.plugins */
    lua_pushvalue(L, -1);
    lua_helper_getfield_siginfo_ng(L, "plugins");

    lua_insert(L, -2);
    lua_setfield(L, -2, namespace);
    lua_pop(L, 1);

    /* push plugin chunk on the stack */
    if(luaL_loadfile(L, filename) != 0) {
        lua_helper_printerror(L);
    }

    /* keep reference to plugin chunk in namespace */
    lua_pushvalue(L, -1);
    lua_setfield(L, -3, "__code");

    /* set environment for plugin chunk */
    lua_insert(L, -2);
    lua_setfenv(L, -2);

    /* and initialize it */
    lua_pushboolean(L, 1);
    lua_setglobal(L, "__init__");

    lua_helper_callfunction(L, 0, 1);

    lua_pushnil(L);
    lua_setglobal(L, "__init__");

    if(lua_isboolean(L, -1) && !lua_toboolean(L, -1)) {
        lua_helper_getfield_siginfo_ng(L, "plugins");
        lua_pushnil(L);
        lua_setfield(L, -2, namespace);
        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

void lua_plugin_load_dir(lua_State *L, const char *directory) {
    DIR *dir_fd;
    struct dirent *dirent;

    if(!(dir_fd = opendir(directory))) {
        log_print(log_Error,
            "Failed to load plugin directory \"%s\": %s\n",
            directory, strerror(errno)
        );
        return;
    }

    while((dirent = readdir(dir_fd)) != 0) {
        char *filename, *namespace, *ext;
        size_t fn_len, ns_len;

        ext = strrchr(dirent->d_name, '.');
        if(!ext || strcmp(ext, LUA_PLUGIN_EXT) != 0) {
            continue;
        }

        fn_len = strlen(directory) + strlen(dirent->d_name) + 1;
        ns_len = strlen(dirent->d_name) - strlen(ext);

        filename = malloc(fn_len+1);
        namespace = malloc(ns_len+1);

        if(!namespace || !filename) {
            log_print(log_Fatal, "lua_plugin_load_dir: malloc() failed!\n");
            lua_close(L);
            exit(EXIT_FAILURE);
        }

        snprintf(filename, fn_len+1, "%s/%s", directory, dirent->d_name);

        strncpy(namespace, dirent->d_name, ns_len);
        namespace[ns_len] = '\0';

        lua_plugin_load(L, namespace, filename);

        free(filename);
        free(namespace);
    }

    closedir(dir_fd);
}

void lua_plugin_execute(lua_State *L) {
    lua_helper_getfield_siginfo_ng(L, "plugins");
    lua_pushnil(L);

    while( lua_next( L, -2 ) ) {
        /* -1 value, -2 key */
        lua_getfield(L, -1, "__code");
        lua_helper_callfunction(L, 0, 0);

        lua_pop(L, 1);
    }

    lua_pop(L, 1);
}

static int lua_plugin_api_loadplugin(lua_State *L) {
    lua_plugin_load(L, luaL_checkstring(L, 1), luaL_checkstring(L, 2));
    return 0;
}

static int lua_plugin_api_loadfolder(lua_State *L) {
    lua_plugin_load_dir(L, luaL_checkstring(L, 1));
    return 0;
}

static void lua_plugin_api_readfp(lua_State *L, FILE *fp) {
    size_t len;
    char *bufptr;
    luaL_Buffer buf;

    luaL_buffinit(L, &buf);
    bufptr = luaL_prepbuffer(&buf);

    len = fread(bufptr, sizeof(char), LUAL_BUFFERSIZE, fp);

    luaL_addsize(&buf, len);
    luaL_pushresult(&buf);
}

static int lua_plugin_api_readfile(lua_State *L) {
    const char *filename = luaL_checkstring(L, 1);
    FILE *fp = fopen(filename, "r");
    if(fp == NULL) {
        luaL_error(L, "%s: %s\n", filename, strerror(errno));
        return 0;
    }

    lua_plugin_api_readfp(L, fp);

    fclose(fp);

    return 1;
}

static int lua_plugin_api_readexec(lua_State *L) {
    const char *command = luaL_checkstring(L, 1);
    FILE *pd = popen(command, "r");
    if(pd == NULL) {
        luaL_error(L, "%s: %s\n", command, strerror(errno));
        return 0;
    }

    lua_plugin_api_readfp(L, pd);

    if(pclose(pd) == -1) {
        luaL_error(L, "pclose() failed: %s\n", command, strerror(errno));
        return 0;
    }

    return 1;
}

static int lua_plugin_api_using(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);

    lua_helper_getfield_siginfo_ng(L, "template");
    lua_getfield(L, -1, name);

    lua_setglobal(L, name);

    return 0;
}

void lua_plugin_register_api(lua_State *L) {
    lua_register(L, "using", lua_plugin_api_using);

    lua_pushcfunction(L, lua_plugin_api_loadplugin);
    lua_helper_setfield_siginfo_ng(L, "loadplugin");

    lua_pushcfunction(L, lua_plugin_api_loadfolder);
    lua_helper_setfield_siginfo_ng(L, "loadfolder");

    lua_pushcfunction(L, lua_plugin_api_readfile);
    lua_helper_setfield_siginfo_ng(L, "readfile");

    lua_pushcfunction(L, lua_plugin_api_readexec);
    lua_helper_setfield_siginfo_ng(L, "readexec");
}
