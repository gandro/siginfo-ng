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
#include <errno.h>
#include <string.h>
#include <sys/vfs.h>

#include "../plugin.h"
#include "../siginfo-ng.h"

#define GByte (1000*1000*1000)

struct {
    float hdd_total;
    float hdd_free;
    float hdd_used;
    int fscount;
} linux_fsstat;



void update_fsstat() {
    FILE *mounttab;
    struct statfs fsstat;
    char line[512], *mountpoint;
    static int last_updated = -1;

    if(last_updated == profile.last_updated) {
        return;
    }
    last_updated = profile.last_updated;

    mounttab = fopen("/proc/mounts", "r");
    if(mounttab == NULL) {
        fprintf(logfile, 
            "Error: Failed to open /proc/mounts: %s", strerror(errno));
        return;
    }

    linux_fsstat.hdd_total = linux_fsstat.hdd_free = linux_fsstat.hdd_used = 0;
    linux_fsstat.fscount = 0;

    while(fgets(line, sizeof(line), mounttab) != NULL) {
        if(strncmp("/dev/", line, 5) != 0) {
            continue;
        }

        strtok(line, " \t");
        mountpoint = strtok(NULL, " \t");

        if(statfs(mountpoint, &fsstat) != 0) {
            fprintf(logfile, "Error: Failed to call statfs() for '%s': %s",
                                mountpoint, strerror(errno));
            return;
        }

        linux_fsstat.hdd_total +=
            (((double)fsstat.f_blocks)*((double)fsstat.f_bsize))/GByte;
        linux_fsstat.hdd_free +=
            (((double)fsstat.f_bavail)*((double)fsstat.f_bsize))/GByte;
        linux_fsstat.fscount++;
    }
    linux_fsstat.hdd_used = linux_fsstat.hdd_total - linux_fsstat.hdd_free;
}

void fsstat_hdd_total(plugin_t *self) {
    update_fsstat();
    set_value(self, &linux_fsstat.hdd_total, T_FLOAT);
}

void fsstat_hdd_free(plugin_t *self) {
    update_fsstat();
    set_value(self, &linux_fsstat.hdd_free, T_FLOAT);
}

void fsstat_hdd_used(plugin_t *self) {
    update_fsstat();
    set_value(self, &linux_fsstat.hdd_used, T_FLOAT);
}

void fsstat_hdd_fscount(plugin_t *self) {
    update_fsstat();
    set_value(self, &linux_fsstat.fscount, T_INTEGER);
}

void linux_fsstat_init() {
    register_plugin("HDD_TOTAL", fsstat_hdd_total);
    register_plugin("HDD_FREE", fsstat_hdd_free);
    register_plugin("HDD_USED", fsstat_hdd_used);
    register_plugin("HDD_FSCOUNT", fsstat_hdd_fscount);
}
