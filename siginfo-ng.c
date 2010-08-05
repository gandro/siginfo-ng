#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "siginfo-ng.h"

#ifdef USE_GETOPT_LONG
#include <getopt.h>
#endif

void log_print(log_Severity severity, const char *fmt, ...) {
    va_list args;
    const char *prefix = "";

    switch(severity) {
        case log_Notice:
            prefix = "Notice: ";
            break;
        case log_Warning:
            prefix = "Warning: ";
            break;
        case log_Error:
            prefix = "Error: ";
            break;
        case log_Fatal:
            prefix = "Fatal: ";
            break;
    }

    fprintf(stderr, prefix);

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}


static void log_redirect(const char *logfile)  {
    FILE *logfd = fopen(logfile, "a");
    if(logfd == NULL) {
        log_print(log_Error, "Failed to open logfile: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    setvbuf(logfd, NULL, _IONBF, BUFSIZ);
    dup2(fileno(logfd), STDERR_FILENO);
    fclose(logfd);
}

static const char *client_find_configfile() {
    static const char *defaults[] = {
        "~/.siginfo-ng/config.lua",
        "/etc/siginfo-ng/config.lua",
        "./config.lua",
        NULL
    };
    const char **configfile = defaults;

    while(*configfile) {
        if(access(*configfile, R_OK) == 0) {
            return *configfile;
        }

        configfile++;
    }

    log_print(log_Error, "Could not find config file!\n");
    exit(EXIT_FAILURE);
    return NULL;
}

static int client_update(lua_State *L, siginfo_Settings *settings) {
    int status;
    const char *message;

    lua_settings_parse(L, settings);

    status = siginfo_publish_data(settings);
    message = siginfo_status_message(status);

    lua_pushinteger(L, status);
    lua_helper_setfield_siginfo_ng(L, "status");
    lua_pushstring(L, message);
    lua_helper_setfield_siginfo_ng(L, "message");

    if(status != 100) {
        log_print(log_Error, "Server \"%s\" - %s\n", settings->server, message);
    }

    return (status == 100);
}

static void client_show_layout(lua_State *L, siginfo_Settings *settings) {
    int i;

    lua_settings_parse(L, settings);

    for(i=0; i<SIGINFO_ROWS; i++) {
        printf("%s\n", settings->layout.row[i]);
    }
}

static int client_start_deamon(lua_State *L, siginfo_Settings *settings) {
    pid_t pid;

    if(!client_update(L, settings)) {
        return EXIT_FAILURE;
    }

    pid = fork();
    if(pid == 0) {
        int nullfd = open("/dev/null", O_RDWR, 0);
        if(nullfd > 0) {
            dup2(nullfd, STDIN_FILENO);
            dup2(nullfd, STDOUT_FILENO);
            close(nullfd);
        }

        while(sleep(settings->interval) == 0) {
            client_update(L, settings);
        }
    } else if(pid < 0) {
        log_print(log_Fatal, "client_start_deamon: %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void client_print_version() {
    printf(CLIENT_NAME " " CLIENT_VERSION "\n");
    exit(EXIT_SUCCESS);
}

static void client_print_help() {
    printf("Usage:\n\n"

            "    " CLIENT_NAME " [--show | --daemon | --help | --version]\n"
            "               [--config=CONFIGFILE] [--log=LOGFILE]\n\n"

            "Available Options:\n\n"

            "        -s, --show          Show Siginfo layout and quit\n"
            "        -d, --daemon        Run as daemon\n\n"

            "        -c, --config=FILE   Load settings from file\n"
            "        -l, --logfile=FILE  Write error messages to logfile\n\n"

            "        -h, --help          Show this help and quit\n"
            "        -V, --version       Show version number an quit\n\n"

            "For more help see also the comments in configuration file.\n"
            "Submit bugs or feedback to Sebastian Wicki <gandro@gmx.net>.\n"
    );
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    lua_State *L;
    siginfo_Settings settings;
    const char *configfile = NULL, *logfile = NULL;
    enum { action_Show, action_Daemon, action_Update } action = action_Update;
    int exit, result, arguments = 0;
#ifdef USE_GETOPT_LONG
    static const struct option long_options[] = {
        { "show",      no_argument,       0, 's' },
        { "daemon",    no_argument,       0, 'd' },
        { "config",    required_argument, 0, 'c' },
        { "logfile",   required_argument, 0, 'l' },
        { "help",      no_argument,       0, 'h' },
        { "version",   no_argument,       0, 'v' },
        { NULL,        0,              NULL,  0  }
    };
#endif

    while (optind < argc) {
#ifdef USE_GETOPT_LONG
        result = getopt_long(argc, argv, "sdc:l:hv", long_options, NULL);
#else
        result = getopt(argc, argv, "sdc:l:hv-");
#endif
        arguments++;

        switch(result) {
            case 's': /* show */
                action = action_Show;
                break;
            case 'd': /* list */
                action = action_Daemon;
                break;
            case 'c': /* config */
                configfile = optarg;
                break;
            case 'l': /* logfile */
                configfile = optarg;
                break;
            case 'v': /* version */
                client_print_version();
                break;
            case 'h': /* help */
                client_print_help();
                break;
#ifndef USE_GETOPT_LONG
            case '-': /* long option */
                log_print(log_Notice,
                    "Long options are not available on your platform!\n");
                exit(EXIT_FAILURE);
                break;
#endif
            default: /* unknown */
                break;
        }
    }

    if(logfile != NULL) {
        log_redirect(logfile);
    }

    if(configfile == NULL) {
        configfile = client_find_configfile();
    }

    exit = EXIT_SUCCESS;
    L = lua_helper_initstate();
    lua_settings_loadfile(L, configfile);

    switch(action) {
        case action_Update:
            exit = client_update(L, &settings);
            break;
        case action_Show:
            client_show_layout(L, &settings);
            break;
        case action_Daemon:
            exit = client_start_deamon(L, &settings);
            break;
    }

    lua_close(L);
    return exit;
}
