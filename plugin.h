/*
 * ----------------------------------------------------------------------------
 * "THE COFFEE-WARE LICENSE" (Revision 12/2007):
 * Sebastian Wicki <gandro@gmx.net> wrote this file. As long as you retain
 * this notice you can do whatever you want with this stuff. If we meet some
 * day, and you think this stuff is worth it, you can buy me a cup of coffee
 * in return. 
 * ----------------------------------------------------------------------------
 */

#ifndef PLUGIN_H
#define PLUGIN_H

typedef struct Plugin {
    char *name;
    char *value;
    void (*function)(struct Plugin *self);
} plugin_t;

typedef enum ValueType {
    T_STRING,
    T_INTEGER,
    T_LONG,
    T_FLOAT
} type_t;

extern plugin_t plugin[64];
extern size_t plugin_s;

extern void init_plugins();
void clear_plugins();
void print_plugins();

void register_plugin(const char *name, void (*func)(plugin_t *self));

plugin_t *find_plugin(const char *name);
int cmp_plugin();
int chk_plugin();

void set_value(plugin_t *plugin, const void *value, type_t type);
char *get_value(const char *value);

#endif /* PLUGIN_H */
