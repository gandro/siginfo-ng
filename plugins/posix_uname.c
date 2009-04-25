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
#include <sys/utsname.h>

#include "../plugin.h"
#include "../siginfo-ng.h"

struct utsname posix_uname;

void uname_sysname(plugin_t *self) {
    set_value(self, posix_uname.sysname, T_STRING);
}

void uname_nodename(plugin_t *self) {
    set_value(self, posix_uname.nodename, T_STRING);
}

void uname_release(plugin_t *self) {
    set_value(self, posix_uname.release, T_STRING);
}

void uname_version(plugin_t *self) {
    set_value(self, posix_uname.version, T_STRING);
}

void uname_machine(plugin_t *self) {
    set_value(self, posix_uname.machine, T_STRING);
}

void posix_uname_init() {
    if(uname(&posix_uname) != 0) {
        fprintf(logfile, 
            "Error: Failed to call uname(): %s", strerror(errno));
        return;
    }

    register_plugin("UNAME_SYSNAME", uname_sysname);
    register_plugin("UNAME_NODENAME", uname_nodename);
    register_plugin("UNAME_RELEASE", uname_release);
    register_plugin("UNAME_VERSION", uname_version);
    register_plugin("UNAME_MACHINE", uname_machine);
}
