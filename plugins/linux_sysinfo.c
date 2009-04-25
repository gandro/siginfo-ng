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
#include <sys/sysinfo.h>

#include "../plugin.h"
#include "../siginfo-ng.h"

#define MByte (1000*1000)

struct {
    float cpu_load_1m;
    float cpu_load_5m;
    float cpu_load_15m;
    int processes;

    int ram_total;
    int ram_free;
    int ram_used;

    int swap_total;
    int swap_free;
    int swap_used;
} linux_sysinfo;

void update_sysinfo() {
    struct sysinfo sysinfo_st;
    static int last_updated = -1;

    if(last_updated == profile.last_updated) {
        return;
    }
    last_updated = profile.last_updated;

    if(sysinfo(&sysinfo_st) != 0) {
        fprintf(logfile, 
            "Error: Failed to call sysinfo(): %s", strerror(errno));
        return;
    }

    linux_sysinfo.cpu_load_1m = sysinfo_st.loads[0]/(float)(1<<SI_LOAD_SHIFT);
    linux_sysinfo.cpu_load_5m = sysinfo_st.loads[1]/(float)(1<<SI_LOAD_SHIFT);
    linux_sysinfo.cpu_load_15m = sysinfo_st.loads[2]/(float)(1<<SI_LOAD_SHIFT);

    linux_sysinfo.processes = (int) sysinfo_st.procs;

    linux_sysinfo.ram_total = (int) (sysinfo_st.totalram/MByte);
    linux_sysinfo.ram_free = (int) (sysinfo_st.freeram/MByte);
    linux_sysinfo.ram_used =
        linux_sysinfo.ram_total - linux_sysinfo.ram_free;

    linux_sysinfo.swap_total = (int) (sysinfo_st.totalswap/MByte);
    linux_sysinfo.swap_free = (int) (sysinfo_st.freeswap/MByte);
    linux_sysinfo.swap_used =
        linux_sysinfo.swap_total - linux_sysinfo.swap_free;
}

#define create_sysinfo_hook(unit) \
void sysinfo_##unit(plugin_t *self) { \
    update_sysinfo(); \
    set_value(self, &linux_sysinfo.unit, T_INTEGER); \
}

void sysinfo_cpu_load_1m(plugin_t *self) {
    update_sysinfo();
    set_value(self, &linux_sysinfo.cpu_load_1m, T_FLOAT);
}

void sysinfo_cpu_load_5m(plugin_t *self) {
    update_sysinfo();
    set_value(self, &linux_sysinfo.cpu_load_5m, T_FLOAT);
}

void sysinfo_cpu_load_15m(plugin_t *self) {
    update_sysinfo();
    set_value(self, &linux_sysinfo.cpu_load_15m, T_FLOAT);
}

create_sysinfo_hook(processes);

create_sysinfo_hook(ram_total);
create_sysinfo_hook(ram_free);
create_sysinfo_hook(ram_used);

create_sysinfo_hook(swap_total);
create_sysinfo_hook(swap_free);
create_sysinfo_hook(swap_used);

void linux_sysinfo_init() {
    register_plugin("LOADAVG_01", sysinfo_cpu_load_1m);
    register_plugin("LOADAVG_05", sysinfo_cpu_load_5m);
    register_plugin("LOADAVG_15", sysinfo_cpu_load_15m);

    register_plugin("PROCESSES", sysinfo_processes);

    register_plugin("RAM_TOTAL", sysinfo_ram_total);
    register_plugin("RAM_FREE", sysinfo_ram_free);
    register_plugin("RAM_USED", sysinfo_ram_used);

    register_plugin("SWAP_TOTAL", sysinfo_swap_total);
    register_plugin("SWAP_FREE", sysinfo_swap_free);
    register_plugin("SWAP_USED", sysinfo_swap_used);
}
