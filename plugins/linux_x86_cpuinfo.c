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
#include <stdlib.h>

#include "../plugin.h"
#include "../siginfo-ng.h"

#define MAX_CPUS 16

unsigned int cpu_s = 0;

struct {
    char model[64];
    float speed_mhz;
    int cache_kb;
} cpu[MAX_CPUS];


unsigned int get_cpu_by_hook(const char *hook_name) {
    if(strncmp("CPU", hook_name, 3) == 0) {
        hook_name += 3;
        return (unsigned int) (atoi(hook_name) % cpu_s);
    }
    return 0;
}

void cpuinfo_model(plugin_t *self) {
    unsigned int cpu_n = get_cpu_by_hook(self->name);
    set_value(self, cpu[cpu_n].model, T_STRING);
}

void cpuinfo_mhz(plugin_t *self) {
    unsigned int cpu_n = get_cpu_by_hook(self->name);
    set_value(self, &cpu[cpu_n].speed_mhz, T_FLOAT);
}

void cpuinfo_cache(plugin_t *self) {
    unsigned int cpu_n = get_cpu_by_hook(self->name);
    set_value(self, &cpu[cpu_n].cache_kb, T_INTEGER);
}

void cpuinfo_count(plugin_t *self) {
    set_value(self, &cpu_s, T_INTEGER);
}

void linux_x86_cpuinfo_init() {
    FILE *cpuinfo_d;
    char line[512], *value = NULL;
    unsigned int i, cpu_n;

    cpuinfo_d = fopen("/proc/cpuinfo", "r");
    if(cpuinfo_d == NULL) {
        fprintf(logfile, 
            "Error: Failed to open /proc/cpuinfo: %s", strerror(errno));
        return;
    }

    while(fgets(line, sizeof(line), cpuinfo_d) != NULL) {
        if((value = strchr(line, ':')) == NULL) {
            continue;
        }

        line[strlen(line)-1] = '\0';
        value += 2;

        if(strncmp(line, "processor", 9) == 0) {
            cpu_n = atoi(value);

            if(cpu_n >= MAX_CPUS) {
                cpu_n = MAX_CPUS-1;
                break;
            }

        } else if(strncmp(line, "model name", 10) == 0) {
            strncpy(cpu[cpu_n].model, value, sizeof(cpu[cpu_n].model));            cpu[cpu_n].model[sizeof(cpu[cpu_n].model)-1] = '\0';
        } else if(strncmp(line, "cpu MHz", 7) == 0) {
            cpu[cpu_n].speed_mhz = atof(value);
        } else if(strncmp(line, "cache size", 10) == 0) {
            cpu[cpu_n].cache_kb = atoi(value);
        }
    }

    cpu_s = cpu_n+1;
    for(i=0; i<cpu_s; i++) {
        char hook[32];

        snprintf(hook, sizeof(hook), "CPU%i_MODEL", i);
        register_plugin(hook, cpuinfo_model);

        snprintf(hook, sizeof(hook), "CPU%i_MHZ", i);
        register_plugin(hook, cpuinfo_mhz);

        snprintf(hook, sizeof(hook), "CPU%i_CACHE", i);
        register_plugin(hook, cpuinfo_cache);
    }

    register_plugin("CPU_COUNT", cpuinfo_count);
}
