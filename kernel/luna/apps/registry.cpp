#include "app.hpp"
#include "../desktop/startmenu/startmenu.hpp"

Wm *g_luna_wm = nullptr;

static void trampoline(void *user)
{
    auto *app = (LunaApp *)user;
    if (app && app->open)
        app->open(app->user);
}

// openers declarations
void app_about_open(void *user);
void app_terminal_open(void *user);

static LunaApp g_apps[] = {
    {"about", "About Luna", "Apps", app_about_open, nullptr},
    {"terminal", "Terminal", "System", app_terminal_open, nullptr},
};

void luna_apps_register_menu(StartMenu &menu)
{
    for (unsigned i = 0; i < sizeof(g_apps) / sizeof(g_apps[0]); i++)
        menu.add_item(g_apps[i].name, g_apps[i].category,
                      trampoline, &g_apps[i]);
}
