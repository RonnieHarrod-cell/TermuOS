#pragma once

struct LunaApp
{
    const char *id;
    const char *name;
    const char *category;
    void (*open)(void *user);
    void *user;
};

class Wm;
extern Wm *g_luna_wm;

void luna_apps_register_menu(class StartMenu &menu);
