#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "siginfo-ng.h"

void lua_settings_loadfile(lua_State *L, const char *configfile) {
    char *fullpath;

    if(luaL_loadfile(L, configfile) != 0) {
        lua_helper_printerror(L);
    }

    fullpath = realpath(configfile, NULL);
    if(fullpath != NULL) {
        lua_pushstring(L, fullpath);
        lua_setglobal(L, "__self");

        lua_pushstring(L, dirname(fullpath));
        lua_setglobal(L, "__path");

        free(fullpath);
    }

    lua_helper_callfunction(L, 0, 0);
}

void lua_settings_parse_layout(lua_State *L, siginfo_Layout *layout) {
    char row[5];
    unsigned short i;

    lua_getfield(L, -1, "layout");

    if(!lua_istable(L, -1)) {
        log_print(log_Error, "Missing layout table in configuration!\n");
        lua_close(L);
        exit(EXIT_FAILURE);
    }

    for(i=0; i<SIGINFO_ROWS; i++) {
        memset(layout->row[i], 0, SIGINFO_ROWLEN);
        strcpy(row, "row1");
        row[3] += i;

        lua_getfield(L, -1, row);

        if(!lua_istable(L, -1)) {
            if(lua_type(L, -1) == LUA_TFUNCTION) {
                lua_helper_callfunction(L, 0, 1);
            }

            if(lua_isstring(L, -1)) {
                strncpy(layout->row[i], lua_tostring(L, -1), SIGINFO_ROWLEN - 1);
            }
            lua_pop(L, 1);
            continue;
        }

        lua_pushnil(L); /* first key */
        while(lua_next(L, -2) != 0) {
            const char *value;
               /* key at index -2 and value at index -1 */

            if(lua_type(L, -1) == LUA_TFUNCTION) {
                /* pop function and push result on the stack */
                lua_helper_callfunction(L, 0, 1);
            }

            value = lua_tostring(L, -1);

            if(strlen(layout->row[i]) + strlen(value) >= SIGINFO_ROWLEN) {
                log_print(log_Warning, "Not enough space in row%d!\n", i+1);
                lua_pop(L, 2);
                break;
            }

            strcat(layout->row[i], value);
            /* remove value; keep key for next iteration */
            lua_pop(L, 1);
        }

        /* remove row table from stack */
        lua_pop(L, 1);
    }

     /* remove layout table from stack */
     lua_pop(L, 1);
}


static void lua_settings_parse_uptime(lua_State *L, siginfo_Settings *settings) {
    lua_getfield(L, -1, "uptime");

    if(lua_isfunction(L, -1)) {
        lua_helper_callfunction(L, 0, 1);
    }

    if(lua_isnumber(L, -1)) {
        settings->uptime = lua_tostring(L, -1);
    } else {
        settings->uptime = "0";
        log_print(log_Warning, "Invalid value or function in uptime configuration!\n");
    }

    lua_pop(L, 1);
}

static const char *lua_settings_parse_string(lua_State *L, const char *name) {
    const char *string = "";

    lua_getfield(L, -1, name);

    if(!lua_isstring(L, -1)) {
        log_print(log_Warning,
            "Invalid or missing configuration value for option: %s\n", name);
    } else {
        string = lua_tostring(L, -1);
    }
    lua_pop(L, 1);

    return string;
}

static int lua_settings_parse_integer(lua_State *L, const char *name) {
    int integer = 0;

    lua_getfield(L, -1, name);

    if(!lua_isnumber(L, -1)) {
        log_print(log_Warning,
            "Invalid or missing configuration value for option: %s\n", name);
    }

    integer = lua_tointeger(L, -1);
    lua_pop(L, 1);

    return integer;
}

void lua_settings_parse(lua_State *L, siginfo_Settings *settings) {
    lua_getglobal(L, "siginfo");

    if(!lua_istable(L, -1)) {
        log_print(log_Error,
            "Invalid type for 'siginfo' namespace: %s\n", luaL_typename(L, -1));
    }

    settings->server   = lua_settings_parse_string(L, "server");
    settings->port     = lua_settings_parse_string(L, "port");
    settings->interval = lua_settings_parse_integer(L, "interval");

    settings->username = lua_settings_parse_string(L, "username");
    settings->password = lua_settings_parse_string(L, "password");

    settings->computer = lua_settings_parse_string(L, "computer");

    settings->version = lua_settings_parse_string(L, "version");

    lua_settings_parse_uptime(L, settings);
    lua_settings_parse_layout(L, &settings->layout);

    /* pop siginfo namespace table */
    lua_pop(L, 1);
}

int lua_settings_onupdate_callback(lua_State *L) {
    int doupdate = 1;

    lua_getglobal(L, "siginfo");
    lua_getfield(L, -1, "onupdate");

    if(lua_isfunction(L, -1)) {
        lua_helper_callfunction(L, 0, 1);
        if(lua_isboolean(L, -1)) {
            doupdate = lua_toboolean(L, -1);
        }
    }
    lua_pop(L, 2);

    return doupdate;
}

void lua_settings_onerror_callback(lua_State *L, const char *message, int status) {
    lua_getglobal(L, "siginfo");
    lua_getfield(L, -1, "onerror");

    if(lua_isfunction(L, -1)) {
        lua_pushstring(L, message);
        lua_pushnumber(L, status);
        lua_helper_callfunction(L, 2, 0);
    }

    lua_pop(L, 2);
}
