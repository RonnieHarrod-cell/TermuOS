#include "app.hpp"
#include "../desktop/wm.hpp"
#include "../widgets/window.hpp"
#include "../widgets/label.hpp"
#include "../widgets/button.hpp"
#include "../widgets/textfield.hpp"

static Window widgets_win;
static Label widgets_lab;
static Button widgets_btn;
static TextField widgets_tf;
static bool widgets_ready;

void app_widgets_open(void *user)
{
    (void)user;

    if (!widgets_ready)
    {
        widgets_win.x = 180;
        widgets_win.y = 120;
        widgets_win.w = 520;
        widgets_win.h = 340;
        widgets_win.title = "Luna Widgets";
        widgets_win.visible = false;

        widgets_lab.x = 16;
        widgets_lab.y = 48;
        widgets_lab.w = 200;
        widgets_lab.h = 30;
        widgets_lab.text = "Luna Widgets";
        widgets_win.add(&widgets_lab);

        widgets_btn.x = 16;
        widgets_btn.y = 96;
        widgets_btn.w = 70;
        widgets_btn.h = 25;
        widgets_btn.text = "Button";
        widgets_win.add(&widgets_btn);

        widgets_tf.x = 16;
        widgets_tf.y = 144;
        widgets_tf.w = 300;
        widgets_tf.h = 300;
        widgets_win.add(&widgets_tf);

        widgets_ready = true;
        if (g_luna_wm)
            g_luna_wm->add(&widgets_win);
    }

    widgets_win.visible = true;
    if (g_luna_wm)
        g_luna_wm->raise(&widgets_win);
    widgets_win.mark_dirty();
}
