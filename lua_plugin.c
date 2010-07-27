#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "siginfo-ng.h"

void lua_plugin_load(lua_State *L, const char *namespace, const char *filename) {
    /* push plugin chunk on the stack */
    if(luaL_loadfile(L, filename) != 0) {
        lua_helper_printerror(L);
    }

    /* new environment table as global, but keep it on the stack  */
    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, namespace);

    /* load plugin metatable on the stack */
    luaL_getmetatable(L, LUA_PLUGIN_METATABLE);

    /* setmetatable for environment table */
    lua_setmetatable(L, -2);

    /* set environment table for plugin code */
    lua_setfenv(L, -2);

    /* execute plugin code */
    lua_helper_callfunction(L, 0, 0);
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
