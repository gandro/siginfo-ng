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

struct {
    int secs, secs_total;
    int mins, mins_total;
    int hours, hours_total;
    int days, days_total;
    int years, years_total;
} uptime;

void update_uptime() {
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

    uptime.secs_total = (int) sysinfo_st.uptime;
    uptime.secs = uptime.secs_total % 60;

    uptime.mins_total = uptime.secs_total / 60;
    uptime.mins = uptime.mins_total % 60;

    uptime.hours_total = uptime.mins_total / 60;
    uptime.hours = uptime.hours_total % 60;

    uptime.days_total = uptime.hours_total / 24;
    uptime.days = uptime.days_total % 24;

    uptime.years_total = uptime.years_total / 356;
    uptime.years = uptime.years_total % 356;
}

#define create_uptime_hook(unit) \
void uptime_##unit(plugin_t *self) { \
    update_uptime(); \
    set_value(self, &uptime.unit, T_INTEGER); \
}

create_uptime_hook(secs);
create_uptime_hook(secs_total);

create_uptime_hook(mins);
create_uptime_hook(mins_total);

create_uptime_hook(hours);
create_uptime_hook(hours_total);

create_uptime_hook(days);
create_uptime_hook(days_total);

create_uptime_hook(years);
create_uptime_hook(years_total);

void linux_uptime_init() {

    register_plugin("UPTIME_SECS", uptime_secs);
    register_plugin("UPTIME_SECS_TOTAL", uptime_secs_total);

    register_plugin("UPTIME_MINS", uptime_mins);
    register_plugin("UPTIME_MINS_TOTAL", uptime_mins_total);

    register_plugin("UPTIME_HOURS", uptime_hours);
    register_plugin("UPTIME_HOURS_TOTAL", uptime_hours_total);

    register_plugin("UPTIME_DAYS", uptime_days);
    register_plugin("UPTIME_DAYS_TOTAL", uptime_days_total);

    register_plugin("UPTIME_YEARS", uptime_years);
    register_plugin("UPTIME_YEARS_TOTAL", uptime_years_total);

}
