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

#include "siginfo-ng.h"
#include "plugin.h"

plugin_t plugin[64];
size_t plugin_s = 0;

int cmp_plugin(const void *plugin1, const void *plugin2) {
    return strcmp(((plugin_t*) plugin1)->name, ((plugin_t*) plugin2)->name);
}

int chk_plugin(const void *name, const void *plugin) {
    return strcmp(name, ((plugin_t*)plugin)->name);
}

void register_plugin(const char *name, void (*function)(plugin_t *self)) {

    if(find_plugin(name) != NULL) {

        fprintf(logfile, "Warning: Plugin \"%s\" already registered!\n", name);

    } else if(plugin_s >= sizeof(plugin)/sizeof(plugin_t)) {

        fprintf(logfile, "Warning: No memory left for plugin \"%s\" "
                        "(max. %i plugins allowed)\n", 
                        name, (int)(sizeof(plugin)/sizeof(plugin_t)));

    } else {

        plugin[plugin_s].name = strdup(name);
        plugin[plugin_s].value = NULL;
        plugin[plugin_s].function = function;
        plugin_s++;

        qsort(plugin, plugin_s, sizeof(plugin_t), cmp_plugin);

    }
}

void clear_plugins() {
    int i;
    for(i=0; i<plugin_s; i++) {
        free(plugin[i].name);
        free(plugin[i].value);
        plugin[i].function = NULL;
    }

    plugin_s = 0;
}

void print_plugins() {
    int i;
    for(i=0; i<plugin_s; i++) {
        printf("%s\n", plugin[i].name);
    }
}

plugin_t *find_plugin(const char *name) {
    return bsearch(name, plugin, plugin_s, sizeof(plugin_t), chk_plugin);
}

void set_value(plugin_t *plugin, const void *value, type_t type) {
    char buffer[48];

    switch(type) {
        case T_STRING:
            value = (char*) value;
            break;
        case T_INTEGER:
            snprintf(buffer, sizeof(buffer),"%i", *((int*)value));
            buffer[sizeof(buffer)-1] = '\0';
            value = buffer;
            break;
        case T_LONG:
            snprintf(buffer, sizeof(buffer),"%li", *((long*)value));
            buffer[sizeof(buffer)-1] = '\0';
            value = buffer;
            break;
        case T_FLOAT:
            snprintf(buffer, sizeof(buffer),"%.2f", *((float*)value));
            buffer[sizeof(buffer)-1] = '\0';
            value = buffer;
            break;
    }

    if(plugin != NULL && value != NULL) {
        plugin->value = realloc(plugin->value, strlen(value)+1);
        if(plugin->value == NULL) {
            fprintf(logfile, "Error: realloc() failed!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(plugin->value, value);
    }
}

char *get_value(const char *name) {
    plugin_t *result = find_plugin(name);

    return (result != NULL) ? result->value : NULL;
}
