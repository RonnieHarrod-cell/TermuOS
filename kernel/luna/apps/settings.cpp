#include "app.hpp"
#include "../desktop/wm.hpp"
#include "../widgets/window.hpp"
#include "../widgets/label.hpp"
#include "../widgets/button.hpp"
#include "../theme.hpp"

extern "C"
{
#include "../../lib/printf.h"
}

static Window set_win;
static Label set_title;
static Label set_info;
static Button btn_teal;
static Button btn_gray;
static bool set_ready;

static void apply_desktop_teal(void *user)
{
    (void)user;
    extern void luna_set_wallpaper(uint32_t colour);
    luna_set_wallpaper(Theme::desktop);
    kprintf("settings: wallpaper teal\n");
}

static void apply_desktop_gray(void *user)
{
    (void)user;
    extern void luna_set_wallpaper(uint32_t colour);
    luna_set_wallpaper(0xFF808080u);
    kprintf("settings: wallpaper gray\n");
}

void app_settings_open(void *user)
{
    (void)user;

    if (!set_ready)
    {
        set_win.x = 120;
        set_win.y = 80;
        set_win.w = 360;
        set_win.h = 220;
        set_win.title = "Settings";
        set_win.visible = false;

        set_title.x = 16;
        set_title.y = 40;
        set_title.w = 320;
        set_title.h = 20;
        set_title.text = "Display";
        set_win.add(&set_title);

        set_info.x = 16;
        set_info.y = 64;
        set_info.w = 320;
        set_info.h = 20;
        set_info.text = "Desktop background";
        set_win.add(&set_info);

        btn_teal.x = 16;
        btn_teal.y = 100;
        btn_teal.w = 120;
        btn_teal.h = 28;
        btn_teal.text = "Teal";
        btn_teal.on_click = apply_desktop_teal;
        set_win.add(&btn_teal);

        btn_gray.x = 150;
        btn_gray.y = 100;
        btn_gray.w = 120;
        btn_gray.h = 28;
        btn_gray.text = "Gray";
        btn_gray.on_click = apply_desktop_gray;
        set_win.add(&btn_gray);

        set_ready = true;
        if (g_luna_wm)
            g_luna_wm->add(&set_win);
    }

    set_win.visible = true;
    if (g_luna_wm)
        g_luna_wm->raise(&set_win);
    set_win.mark_dirty();
}
