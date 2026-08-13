#include "app.hpp"
#include "../desktop/wm.hpp"
#include "../widgets/window.hpp"
#include "../widgets/events.hpp"
#include "../focus.hpp"

extern "C"
{
#include "../../drivers/video/terminal.h"
#include "../../lib/printf.h"
}

static Window term_win;
static bool term_ready;

static void term_layout_client()
{
    int cx = term_win.x + Window::kBorder;
    int cy = term_win.y + Window::kBorder + Window::kTitleH;
    int cw = term_win.w - Window::kBorder * 2;
    int ch = term_win.h - Window::kBorder * 2 - Window::kTitleH;

    terminal_set_offset((uint64_t)cx, (uint64_t)cy);
    terminal_set_size((uint64_t)cw, (uint64_t)ch);
}

void app_terminal_open(void *user)
{
    (void)user;

    if (!term_ready)
    {
        term_win.x = 80;
        term_win.y = 60;
        term_win.w = 520;
        term_win.h = 340;
        term_win.title = "Terminal";
        term_win.visible = false;
        term_ready = true;
        if (g_luna_wm)
            g_luna_wm->add(&term_win);
    }

    term_win.visible = true;
    if (g_luna_wm)
        g_luna_wm->raise(&term_win);

    term_layout_client();
    terminal_putchar('\f');
    kprintf("TermuOS Luna Terminal\n");

    term_win.mark_dirty();
}

bool app_terminal_is_open(void)
{
    return term_ready && term_win.visible;
}

void app_terminal_paint(void)
{
    if (!app_terminal_is_open())
        return;
    term_layout_client();
    terminal_redraw();
}

Window *app_terminal_window(void)
{
    return term_ready ? &term_win : nullptr;
}
