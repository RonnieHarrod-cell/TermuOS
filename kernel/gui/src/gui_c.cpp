#include "../include/gui_c.h"
#include "../include/GUI.hpp"
#include "../include/Window.hpp"
#include "../../drivers/input/mouse.h"
extern "C"
{
#include "../../sched/scheduler.h"
}

static GUI g_gui;
static Window g_test_win(120, 80, 480, 320, "hello termuos");
static bool g_window_added = false;

extern "C" void gui_init(struct limine_framebuffer *fb)
{
    g_gui.init(fb);
    mouse_set_bounds((int)fb->width, (int)fb->height);
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

extern "C" void gui_set_mouse_pos(int x, int y)
{
    g_gui.set_mouse(x, y);
}

extern "C" void gui_handle_mouse(void)
{
    mouse_state_t st;
    mouse_get_state(&st);
    g_gui.set_mouse(st.x, st.y);
    g_gui.update(); // full redraw for now
}

extern "C" void gui_thread_entry(void)
{
    for (;;)
    {
        if (mouse_is_dirty())
            gui_handle_mouse();
        scheduler_yield();
    }
}
