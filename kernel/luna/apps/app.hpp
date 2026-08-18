#pragma once

#include <stdint.h>

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
void app_widgets_open(void *user);
void app_settings_open(void *user);
void app_terminal_open(void *user);
bool app_terminal_is_open(void);
void app_terminal_paint(void);
class Window;
Window *app_terminal_window(void);
void app_terminal_handle_key(char c);

void luna_set_wallpaper(uint32_t colour);
