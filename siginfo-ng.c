#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/stat.h>

#include "siginfo-ng.h"

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

static void client_update(lua_State *L, siginfo_Settings *settings) {
    lua_settings_parse(L, settings);

    if(lua_settings_onupdate_callback(L)) {
        int status = siginfo_publish_data(settings);
        if(status != 100) {
            const char *message = siginfo_status_message(status);
            lua_settings_onerror_callback(L, message, status);
            log_print(log_Error, "Server \"%s\" - %s\n", settings->server, message);
        }
    }
}

static void client_show_layout(lua_State *L, siginfo_Settings *settings) {
    int i;

    lua_settings_parse(L, settings);
    lua_settings_onupdate_callback(L);

    for(i=0; i<SIGINFO_ROWS; i++) {
        printf("%s\n", settings->layout.row[i]);
    }
}

static void client_start_deamon(lua_State *L, siginfo_Settings *settings) {
    pid_t pid = fork();

    if(pid == 0) {
        int nullfd = open("/dev/null", O_RDWR, 0);
        if(nullfd > 0) {
            dup2(nullfd, STDIN_FILENO);
            dup2(nullfd, STDOUT_FILENO);
            close(nullfd);
        }

        do {
            client_update(L, settings);
        } while(sleep(settings->interval) == 0);
    } else if(pid < 0) {
        log_print(log_Fatal, "client_start_deamon: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
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
    int result, arguments = 0;
    static const struct option long_options[] = {
        { "show",      no_argument,       0, 's' },
        { "daemon",    no_argument,       0, 'd' },
        { "config",    required_argument, 0, 'c' },
        { "logfile",   required_argument, 0, 'l' },
        { "help",      no_argument,       0, 'h' },
        { "version",   no_argument,       0, 'v' },
        { NULL,        0,              NULL,  0  }
    };

    while (optind < argc) {
        result = getopt_long(argc, argv, "sdc:l:hv", long_options, NULL);
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

    L = lua_helper_initstate();
    lua_settings_loadfile(L, configfile);

    switch(action) {
        case action_Update:
            client_update(L, &settings);
            break;
        case action_Show:
            client_show_layout(L, &settings);
            break;
        case action_Daemon:
            client_start_deamon(L, &settings);
            break;
    }

    lua_close(L);
    return EXIT_SUCCESS;
}
