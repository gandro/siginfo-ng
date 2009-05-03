/*
 * ----------------------------------------------------------------------------
 * "THE COFFEE-WARE LICENSE" (Revision 12/2007):
 * Sebastian Wicki <gandro@gmx.net> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day, and you think this stuff is worth it, you can buy me a cup of coffee
 * in return. 
 * ----------------------------------------------------------------------------
 */

#ifndef SIGINFONG_H
#define SIGINFONG_H

#include <time.h>

#define TOKEN_MAXLEN 128
#define ROW_MAXVARS 48

#define CLIENT  "siginfo-ng"
#define VERSION "0.1.3"

#define PIDFILE    "/var/run/siginfo-ng.pid"
#define INTERVAL   600
#define CONFIGFILE "/etc/siginfo-ng.conf"

typedef struct {
    enum section_type {
        SECTION_TYPE_STRING,
        SECTION_TYPE_PLUGIN,
        SECTION_TYPE_NULL,
    } type;
    void *data;
} section_t;

typedef struct {
    char username[51];
    char password[41];
    char computer[51];
    section_t row[5][ROW_MAXVARS];
    time_t last_updated;
} siginfo_profile_t;

extern siginfo_profile_t profile;
extern FILE *logfile;
extern char *configfile;

void load_config();
void parse_row(char *src, unsigned int row_n);
char *update_row(unsigned int row_n);

void init_profile();
void clear_profile();

void print_siginfo_status(int status);
void send_siginfo_data();
void print_siginfo_data();

void cleanup();
void print_help();

#endif /* SIGINFONG_H */
