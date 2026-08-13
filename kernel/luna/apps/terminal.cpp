#include "app.hpp"
#include "../desktop/wm.hpp"
#include "../widgets/window.hpp"
#include "../widgets/events.hpp"
#include "../focus.hpp"

extern "C"
{
#include "../../drivers/video/terminal.h"
#include "../../lib/printf.h"
#include "../../shell/shell.h"
}

static Window term_win;
static bool term_ready;

static char line_buf[256];
static int line_len;

static int last_cx = -1, last_cy = -1, last_cw = -1, last_ch = -1;

static void term_prompt(void)
{
    print_prompt();
}

static void term_layout_client()
{
    int cx = term_win.x + Window::kBorder;
    int cy = term_win.y + Window::kBorder + Window::kTitleH;
    int cw = term_win.w - Window::kBorder * 2;
    int ch = term_win.h - Window::kBorder * 2 - Window::kTitleH;

    terminal_set_offset((uint64_t)cx, (uint64_t)cy);

    if (cx != last_cx || cy != last_cy || cw != last_cw || ch != last_ch)
    {
        terminal_set_size((uint64_t)cw, (uint64_t)ch);
        last_cx = cx;
        last_cy = cy;
        last_cw = cw;
        last_ch = ch;
    }
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
    line_len = 0;
    kprintf("TermuOS Luna Terminal\n");
    term_prompt();

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

void app_terminal_handle_key(char c)
{
    if (!app_terminal_is_open())
        return;

    if (c == '\r')
        c = '\n';

    if (c == '\n')
    {
        terminal_putchar('\n');
        line_buf[line_len] = '\0';
        if (line_len > 0)
            shell_run_command(line_buf);
        line_len = 0;
        term_prompt();
        app_terminal_paint();
        return;
    }

    if (c == '\b' || c == 127)
    {
        if (line_len > 0)
        {
            line_len--;
            terminal_putchar('\b');
        }
        app_terminal_paint();
        return;
    }

    if (c >= 32 && c < 127 && line_len < (int)sizeof(line_buf) - 1)
    {
        line_buf[line_len++] = c;
        terminal_putchar(c);
        app_terminal_paint();
    }
}
