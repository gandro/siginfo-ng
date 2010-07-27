#ifndef _SIGINFO_NG_H
#define _SIGINFO_NG_H

#include <lua.h>

#define CLIENT_NAME     "siginfo-ng"
#define CLIENT_VERSION  "0.2.0pre1"

#define SIGINFO_SERVER      "siginfo.de"
#define SIGINFO_PORT        80
#define SIGINFO_INTERVAL    600
#define SIGINFO_COMPUTER    "default-pc"

#define SIGINFO_ROWS     5
#define SIGINFO_ROWLEN   250

#define LUA_PLUGIN_EXT        ".lua"
#define LUA_PLUGIN_METATABLE  "siginfo_Lua_Plugin.metatable"

typedef struct {
    char row[SIGINFO_ROWS][SIGINFO_ROWLEN];
} siginfo_Layout;

typedef struct {
    const char *server;
    const char *port;

    const char *username;
    const char *password;
    const char *computer;

    const char *uptime;
    const char *version;

    siginfo_Layout layout;

    unsigned int interval;
} siginfo_Settings;

typedef enum { log_Notice, log_Warning, log_Error, log_Fatal } log_Severity;

void log_print(log_Severity severity, const char *fmt, ...);

lua_State *lua_helper_initstate();
void lua_helper_printerror(lua_State *L);
void lua_helper_callfunction(lua_State *L, int nargs, int nresults);

void lua_plugin_load(lua_State *L, const char *namespace, const char *filename);
void lua_plugin_load_dir(lua_State *L, const char *directory);

void lua_settings_loadfile(lua_State *L, const char *configfile);
void lua_settings_parse(lua_State *L, siginfo_Settings *settings);
void lua_settings_parse_layout(lua_State *L, siginfo_Layout *layout);
int lua_settings_onupdate_callback(lua_State *L);
void lua_settings_onerror_callback(lua_State *L, const char *message, int status);

int siginfo_publish_data(siginfo_Settings *settings);
const char *siginfo_status_message(int status);

#endif /* _SIGINFO_NG_H */
