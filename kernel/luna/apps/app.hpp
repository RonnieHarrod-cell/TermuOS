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

void app_about_open(void *user);
void app_terminal_open(void *user);
bool app_terminal_is_open(void);
void app_terminal_paint(void);
class Window;
Window *app_terminal_window(void);
