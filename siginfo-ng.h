#ifndef _SIGINFO_NG_H
#define _SIGINFO_NG_H

#define CLIENT_NAME     "siginfo-ng"
#define CLIENT_VERSION  "0.2.0pre4"

#define SIGINFO_SERVER      "siginfo.de"
#define SIGINFO_PORT        80
#define SIGINFO_INTERVAL    600
#define SIGINFO_COMPUTER    "default-pc"

#define SIGINFO_ROWS     5
#define SIGINFO_ROWLEN   250

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

#include <lua.h>

lua_State *lua_helper_initstate();
void lua_helper_printerror(lua_State *L);
void lua_helper_callfunction(lua_State *L, int nargs, int nresults);
void lua_helper_getfield_siginfo_ng(lua_State *L, const char *field);
void lua_helper_setfield_siginfo_ng(lua_State *L, const char *field);

void lua_plugin_load(lua_State *L, const char *namespace, const char *filename);
void lua_plugin_load_dir(lua_State *L, const char *directory);
void lua_plugin_execute(lua_State *L);
void lua_plugin_register_api(lua_State *L);
void lua_plugin_refresh(lua_State *L);

void lua_settings_load_defaults(lua_State *L);
void lua_settings_loadfile(lua_State *L, const char *configfile);
void lua_settings_parse(lua_State *L, siginfo_Settings *settings);
void lua_settings_parse_layout(lua_State *L, siginfo_Layout *layout);

int siginfo_publish_data(siginfo_Settings *settings);
const char *siginfo_status_message(int status);

#define LUA_PLUGIN_EXT        ".lua"
#define LUA_PLUGIN_NAMESPACE  "siginfo_Lua_Plugin_MT_Namespace"
#define LUA_PLUGIN_COMPOSITE  "siginfo_Lua_Plugin_MT_Composite"
#define LUA_PLUGIN_GLOBALACC  "siginfo_Lua_Plugin_MT_Globalacc"

#endif /* _SIGINFO_NG_H */
