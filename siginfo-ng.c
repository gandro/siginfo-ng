/*
 * ----------------------------------------------------------------------------
 * "THE COFFEE-WARE LICENSE" (Revision 12/2007):
 * Sebastian Wicki <gandro@gmx.net> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day, and you think this stuff is worth it, you can buy me a cup of coffee
 * in return. 
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#include "plugin.h"
#include "http.h"
#include "siginfo-ng.h"

siginfo_profile_t profile;
FILE *logfile = NULL;
char *configfile = NULL;

void load_config() {
    FILE *config;
    char line[512], *value;
    int i,key_len, row_n, line_n = 0;

    config = fopen(configfile, "r");
    if(config == NULL) {
        fprintf(logfile, "Warning: Failed to open config file \"%s\": %s\n",
                        configfile, strerror(errno));
        exit(EXIT_FAILURE);
    }

    memset(line, 0, sizeof(line));

    /* start with empty profile */
    profile.username[0] = profile.password[0] = profile.username[0] ='\0';
    for(i=0; i<5; i++) {
        profile.row[i][0].type = SECTION_TYPE_NULL;
        profile.row[i][0].data = NULL;
    }
    profile.last_updated = 0;

    while(fgets(line, sizeof(line), config) != NULL) {
        line_n++;
        line[strlen(line)-1] = '\0';
        if(line[0] == '#' || line[0] == '\0') continue;

        if((value = strchr(line, '=')) == NULL || value[1] == '\0') {
            fprintf(logfile, "Warning: Invalid line %d in \"%s\"\n", 
                            line_n, configfile);
            continue;
        }

        key_len = (int)(value++ - line);

        if(strncmp("username", line, key_len) == 0) {
            strncpy(profile.username, value, sizeof(profile.username));

        } else if(strncmp("password", line, key_len) == 0) {
            strncpy(profile.password, value, sizeof(profile.password));

        } else if(strncmp("computer", line, key_len) == 0) {
            strncpy(profile.computer, value, sizeof(profile.computer));

        } else if(strncmp("row", line, 3) == 0 && 
                    line[3] >= '1' && line[3] <= '5') {
            row_n = line[3]-'1';
            parse_row(value, row_n);

        } else {
            fprintf(logfile, "Warning: Unkown key \"%s\" in line %d in \"%s\"\n", 
                            strtok(line, "="), line_n, configfile);
        }
    }
    fclose(config);
}

void parse_row(char *src, unsigned int row_n) {
    char buffer[TOKEN_MAXLEN];
    char *token = NULL, *token_end = NULL;
    plugin_t *plugin_p = NULL;
    int token_len = 0, var_n = 0;

    memset(buffer, 0, TOKEN_MAXLEN);
    token = strtok(src, "{");
    do {
        if((token_end = strchr(token, '}')) != NULL) {
            token_len = (token_end - token) % TOKEN_MAXLEN;
            strncpy(buffer, token, token_len);
            buffer[token_len] = '\0';

            if((plugin_p = find_plugin(buffer)) == NULL) {
                fprintf(logfile, "Warning: Unkown plugin hook {%s} in row %d!\n",
                                buffer, row_n+1);
            } else if(var_n+2 < ROW_MAXVARS) {
                profile.row[row_n][var_n].type = SECTION_TYPE_PLUGIN;
                profile.row[row_n][var_n].data = plugin_p;
                var_n++;
            } else {
                fprintf(logfile, "Warning: Too much tokens in row %d!\n",
                                row_n+1);
            }

            token=token_end+1;
        } else if(token != src) {
            fprintf(logfile, "Warning: Missing '}' in row %d, character %d!\n",
                            row_n+1, (int)(token-src));
        }

        if(*token) {
            if(var_n+2 < ROW_MAXVARS) {
                profile.row[row_n][var_n].type = SECTION_TYPE_STRING;
                profile.row[row_n][var_n].data = strdup(token);
                var_n++;
            } else {
                fprintf(logfile, "Warning: Too much tokens in row %d!\n",
                                row_n+1);
            }
        }

    } while((token = strtok(NULL, "{")) != NULL);

    /* terminator */
    profile.row[row_n][var_n].type = SECTION_TYPE_NULL;
    profile.row[row_n][var_n].data = NULL;
}

char *update_row(unsigned int row_n) {
    int i, row_len = 1;
    char *token, *row;
    plugin_t *plugin_p;

    row = strdup("");

    for(i=0; profile.row[row_n][i].type != SECTION_TYPE_NULL; i++) {
        token = NULL;
        switch(profile.row[row_n][i].type) {
            case SECTION_TYPE_STRING:
                token = (char*) profile.row[row_n][i].data;
                break;

            case SECTION_TYPE_PLUGIN:
                plugin_p = (plugin_t*) profile.row[row_n][i].data;
                if(plugin_p->function != NULL && plugin_p->name != NULL) {
                    plugin_p->function(plugin_p);
                    if(plugin_p->value != NULL) {
                        token = plugin_p->value;
                    }
                } else {
                    fprintf(logfile, "Warning: Illegal plugin section "
                                    "in row %i at token %i!\n", row_n+1, i+1);
                    continue;
                }
                break;

            default:
            case SECTION_TYPE_NULL:
                break;
        }

        if(token != NULL) {
            row_len += strlen(token);
            row = realloc(row, row_len);
            if(row != NULL) {
                strcat(row, token);
            } else {
                fprintf(logfile, "Error: realloc() failed!\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    return row;
}

void print_siginfo_status(int status) {
    switch(status) {
        case 100:
            fprintf(logfile, "Notice: Profile successfully updated.\n");
            break;

        case 201:
            fprintf(logfile, "Error: Invalid username.\n");
            break;
        case 202:
            fprintf(logfile, "Error: Invalid password.\n");
            break;
        case 203:
            fprintf(logfile, "Error: Invalid computer.\n");
            break;
        case 204:
            fprintf(logfile, "Error: No username specified.\n");
            break;
        case 205:
            fprintf(logfile, "Error: No password specified.\n");
            break;
        case 206:
            fprintf(logfile, "Error: No computer specified.\n");
            break;
        case 220:
            fprintf(logfile, "Error: Uptime difference to big.\n");
            break;

        case 301:
            fprintf(logfile, "Error: Connection to server failed.\n");
            break;

        case 401:
            fprintf(logfile, "Error: Server couldn't connect to database.\n");
            break;
        case 402:
            fprintf(logfile, "Error: Server reported database error.\n");
            break;
        case 403:
            fprintf(logfile, "Error: Server reported processing error.\n");
            break;

        default:
            fprintf(logfile, "Error: Unknown error (%i).\n", status);
            break;
    }
}

void send_siginfo_data() {
    int i, status = 301;
    char post_data[4096], *post_data_ptr;
    const size_t post_data_s = sizeof(post_data);
    char *row, *f_row;
    plugin_t *uptime;

    memset(post_data, 0, post_data_s);
    if(http_connect() == HTTP_SUCCESS && http_send_headers() == HTTP_SUCCESS) {
        post_data_ptr = post_data;
        post_data_ptr += snprintf(post_data, post_data_s, 
                            "user=%s&pass=%s&compi=%s", 
                            profile.username,
                            profile.password,
                            profile.computer);

        for(i=0; i<5; i++) {
            row = update_row(i);
            f_row = url_encode(row);

            post_data_ptr += snprintf(post_data_ptr, 
                                        post_data_s-strlen(post_data),
                                        "&zeile%c=%s", (char)i+'1', f_row);

            free(row);
            free(f_row);
        }

        if(
            (uptime = find_plugin("UPTIME_SECS_TOTAL")) != NULL &&
            uptime->function != NULL
        ) {
            uptime->function(uptime);
            post_data_ptr += snprintf(post_data_ptr, 
                                        post_data_s-strlen(post_data),
                                        "&uptime=%s", uptime->value);
        }

        http_send_post(post_data);
        status = http_get_siginfo_status();
        profile.last_updated = time(NULL);
        http_close();
    }
    print_siginfo_status(status);
}

void print_siginfo_data() {
    int i;
    char *row;

    for(i=0; i<5; i++) {
        row = update_row(i);
        if(strlen(row) > 0) {
            printf("%s\n", row);
        }
        free(row);
    }
}

void print_help() {
    printf("%s [OPTION...]\n", CLIENT);
    printf("Next Generation Siginfo Client\n");
    printf("\nAvailable Options:\n");
    printf("    -s            Print Siginfo rows, don't send any data\n");
    printf("    -d            Run as daemon\n");
    printf("    -p PIDFILE    Specify pidfile (default: \"%s\")\n", PIDFILE);
    printf("    -i INTERVAL   "
                    "Specify update interval (default: %i)\n", INTERVAL);
    printf("    -k            Terminate running daemon\n");
    printf("    -c FILE       "
                    "Load settings from file (default: \"%s\"\n", CONFIGFILE);
    printf("    -l FILE       "
                    "Write error messages to logfile (default: none)\n");
    printf("    -a            Show available plugins and quit\n");
    printf("    -v            Show version information and quit\n");
    printf("    -h            Show this help and quit\n");
    printf("\nFor more help see also the comments in configuration file.\n");
    printf("Submit bugs or feedback to Sebastian Wicki <gandro@gmx.net>.\n");
}

int main(int argc, char *argv[]) {
    int interval = INTERVAL;
    char *pidfile = PIDFILE;
    void (*siginfo_action)() = send_siginfo_data;
    enum { DAEMON_NONE, DAEMON_START, DAEMON_STOP } daemonize = DAEMON_NONE;
    pid_t pid;
    FILE *pidfile_d;

    logfile = stderr;
    configfile = CONFIGFILE;

     while(1) {
        int c = getopt(argc, argv, "sdp:i:kc:l:avh");

        if (c == -1) {
            break;
        }

        switch(c) {
            case 's':
                siginfo_action = print_siginfo_data;
                break;
            case 'd':
                daemonize = DAEMON_START;
                break;
            case 'p':
                pidfile = optarg;
                break;
            case 'i':
                interval = atoi(optarg);
                break;
            case 'k':
                daemonize = DAEMON_STOP;
                break;
            case 'c':
                configfile = optarg;
                break;
            case 'l':
                logfile = fopen(optarg, "a");
                setvbuf(logfile, NULL, _IONBF, BUFSIZ);
                break;
            case 'a':
                init_plugins();
                print_plugins();
                clear_plugins();
                return EXIT_SUCCESS;
            case 'v':
                printf("%s %s\n", CLIENT, VERSION);
                return EXIT_SUCCESS;
            case 'h':
            default:
                print_help();
                return EXIT_FAILURE;
        }
    }

    switch(daemonize) {
        case DAEMON_START:
            init_plugins();
            load_config();

            pidfile_d = fopen(pidfile, "r");
            if(pidfile_d != NULL) {
                fprintf(stderr,"Error: Daemon already running!?\n");
                return EXIT_FAILURE;
                fclose(pidfile_d);
            } 
            
            pidfile_d = fopen(pidfile, "w");
            if(pidfile_d == NULL) {
                fprintf(stderr,
                    "Error: Failed open pidfile: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }

            pid = fork();
            if(pid == 0) {
                signal(SIGINT, clear_plugins);
                do {
                    send_siginfo_data();
                } while(sleep(interval) == 0);
            } else {
                fprintf(pidfile_d, "%i", pid);
                fclose(pidfile_d);
            }
            break;

        case DAEMON_STOP:
            pidfile_d = fopen(pidfile, "r");
            if(pidfile_d == NULL) {
                fprintf(stderr, 
                    "Error: Failed to open pidfile: %s\n", strerror(errno));
                return EXIT_FAILURE;;
            }
            fscanf(pidfile_d, "%d", &pid);
            if(kill(pid, SIGTERM) < 0) {
                fprintf(stderr, "Error: %s\n", strerror(errno));
                return EXIT_FAILURE;
            }
            fclose(pidfile_d);
            remove(pidfile);
            return EXIT_SUCCESS;

        case DAEMON_NONE:
            init_plugins();
            load_config();
            siginfo_action();
            break;
    }

    clear_plugins();

    fclose(logfile);
    return EXIT_SUCCESS;
}
