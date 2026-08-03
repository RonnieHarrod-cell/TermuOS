#include "../include/gui_c.h"
#include "../include/GUI.hpp"
#include "../include/Window.hpp"

static GUI g_gui;
static Window g_test_win(120, 80, 480, 320, "hello termuos");
static bool g_window_added = false;

extern "C" void gui_init(struct limine_framebuffer *fb)
{
    g_gui.init(fb);
}

extern "C" void gui_update(void)
{
    g_gui.update();
}

extern "C" void gui_add_test_window(void)
{
    if (!g_window_added)
    {
        g_gui.add_window(&g_test_win);
        g_window_added = true;
    }
}
