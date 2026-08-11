#include "app.hpp"
#include "../desktop/wm.hpp"
#include "../widgets/window.hpp"
#include "../widgets/label.hpp"

static Window about_win;
static Label about_lab;
static bool about_ready;

void app_about_open(void *user)
{
    (void)user;

    if (!about_ready)
    {
        about_win.x = 180;
        about_win.y = 120;
        about_win.w = 340;
        about_win.h = 200;
        about_win.title = "About Luna";
        about_win.visible = false;

        about_lab.x = 16;
        about_lab.y = 48;
        about_lab.w = 300;
        about_lab.h = 40;
        about_lab.text = "TermuOS Luna\nIn-kernel desktop";
        about_win.add(&about_lab);

        about_ready = true;
        if (g_luna_wm)
            g_luna_wm->add(&about_win);
    }

    about_win.visible = true;
    if (g_luna_wm)
        g_luna_wm->raise(&about_win);
    about_win.mark_dirty();
}
