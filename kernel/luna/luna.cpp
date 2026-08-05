#include "luna.h"
#include "gfx.hpp"
#include "window.hpp"
#include "label.hpp"
#include "button.hpp"
#include "events.hpp"
#include "focus.hpp"
#include "textfield.hpp"

extern "C"
{
#include "../drivers/input/mouse.h"
#include "../drivers/input/keyboard.h"
#include "../drivers/video/fb.h"
#include "../drivers/video/terminal.h"
#include "../lib/printf.h"
}

static constexpr int CURSOR_W = 12;
static constexpr int CURSOR_H = 18;

static int g_mx, g_my, g_omx, g_omy;
static int g_cursor_saved;
static uint32_t g_cursor_under[CURSOR_W * CURSOR_H];
static bool g_running;

static void cursor_erase(Gfx &g)
{
    if (!g_cursor_saved)
        return;
    for (int row = 0; row < CURSOR_H; row++)
        for (int col = 0; col < CURSOR_W; col++)
        {
            int x = g_omx + col, y = g_omy + row;
            g.put_pixel(x, y, g_cursor_under[row * CURSOR_W + col]);
        }
    g_cursor_saved = 0;
}

static void cursor_draw(Gfx &g)
{
    static const char *shape[] = {
        "X          ",
        "XX         ",
        "X.X        ",
        "X..X       ",
        "X...X      ",
        "X....X     ",
        "X.....X    ",
        "X......X   ",
        "X.......X  ",
        "X.....XXXXX",
        "X..X..X    ",
        "X.X X..X   ",
        "XX  X..X   ",
        "     X..X  ",
        "      XX   ",
    };

    int sw = g.width(), sh = g.height();
    if (g_mx < 0)
        g_mx = 0;
    if (g_my < 0)
        g_my = 0;
    if (sw > CURSOR_W && g_mx > sw - CURSOR_W)
        g_mx = sw - CURSOR_W;
    if (sh > CURSOR_H && g_my > sh - CURSOR_H)
        g_my = sh - CURSOR_H;

    for (int row = 0; row < CURSOR_H; row++)
        for (int col = 0; col < CURSOR_W; col++)
        {
            int x = g_mx + col, y = g_my + row;
            g_cursor_under[row * CURSOR_W + col] = g.get_pixel(x, y);
        }
    g_cursor_saved = 1;
    g_omx = g_mx;
    g_omy = g_my;

    for (int r = 0; r < 15; r++)
        for (int c = 0; c < 11; c++)
        {
            char ch = shape[r][c];
            if (ch == ' ')
                continue;
            g.put_pixel(g_mx + c, g_my + r, ch == 'X' ? 0xFFFFFFu : 0x000000u);
        }
}

static void on_btn(void *user)
{
    (void)user;
    kprintf("luna: button clicked\n");
}

extern "C" void luna_run(void)
{
    Gfx gfx;
    gfx.init_from_fb();
    if (gfx.width() <= 0)
        return;

    mouse_set_bounds(gfx.width(), gfx.height());
    g_mx = gfx.width() / 2;
    g_my = gfx.height() / 2;
    g_running = true;

    Window win;
    win.x = 100;
    win.y = 60;
    win.w = 420;
    win.h = 280;
    win.title = "Luna";

    Label lab;
    lab.x = Window::kBorder + 8;
    lab.y = Window::kBorder + Window::kTitleH + 8;
    lab.w = 300;
    lab.h = 20;
    lab.text = "Hello from Luna widgets";
    lab.fg = 0xFF000000u;
    lab.bg = 0xFFE8E8EEu;
    win.add(&lab);

    Button btn;
    btn.x = Window::kBorder + 8;
    btn.y = Window::kBorder + Window::kTitleH + 40;
    btn.w = 120;
    btn.h = 28;
    btn.text = "Click me";
    btn.on_click = on_btn;
    btn.on_click_user = nullptr;
    win.add(&btn);

    TextField field;
    field.x = Window::kBorder + 8;
    field.y = Window::kBorder + Window::kTitleH + 80;
    field.w = 280;
    field.h = 28;
    win.add(&field);

    auto full_redraw = [&]()
    {
        g_cursor_saved = 0;
        gfx.fill_rect(0, 0, gfx.width(), gfx.height(), 0xFF1A1A2Eu);
        if (win.visible)
            win.paint_tree(gfx);
        cursor_draw(gfx);
    };

    full_redraw();
    kprintf("luna: running — Esc to quit\n");

    uint8_t prev_buttons = 0;

    while (g_running)
    {
        bool full = false;

        if (mouse_is_dirty())
        {
            mouse_state_t st;
            mouse_get_state(&st);

            Event ev{};
            ev.x = st.x;
            ev.y = st.y;
            ev.buttons = st.buttons;

            if (st.buttons != prev_buttons)
            {
                if ((st.buttons & 1) && !(prev_buttons & 1))
                    ev.type = EventType::MouseDown;
                else if (!(st.buttons & 1) && (prev_buttons & 1))
                    ev.type = EventType::MouseUp;
                else
                    ev.type = EventType::MouseMove;
                prev_buttons = st.buttons;
                full = true; /* click may change button/window */
            }
            else
            {
                ev.type = EventType::MouseMove;
            }

            if (ev.type == EventType::MouseDown)
            {
                bool hit = win.visible && win.contains_screen(ev.x, ev.y);
                if (hit)
                {
                    win.on_event(ev);
                }
                else
                {
                    Focus::clear();
                    full = true;
                }
            }
            else
            {
                if (win.visible)
                    win.on_event(ev);
            }

            /* drag / hover handling may mark dirty */
            if (win.visible)
                win.on_event(ev);

            if (win.dirty || lab.dirty || btn.dirty)
                full = true;

            if (full)
            {
                win.dirty = lab.dirty = btn.dirty = false;
                g_cursor_saved = false;
                gfx.fill_rect(0, 0, gfx.width(), gfx.height(), 0xFF1A1A2Eu);
                if (win.visible)
                    win.paint_tree(gfx);
                g_mx = st.x;
                g_my = st.y;
                cursor_draw(gfx);
            }
            else
            {
                /* mouse move only — no clear */
                cursor_erase(gfx);
                g_mx = st.x;
                g_my = st.y;
                cursor_draw(gfx);
            }
        }

        if (keyboard_haschar())
        {
            char c = keyboard_getchar();
            if (c == 27)
            {
                g_running = false;
                continue;
            }

            Event kev{};
            kev.type = EventType::KeyDown;
            kev.key = c;
            kev.x = g_mx;
            kev.y = g_my;

            Widget *f = Focus::current();
            if (f)
                f->on_event(kev);
            else if (win.visible)
                win.on_event(kev);

            if (f && true)
                full_redraw();
        }
    }

    terminal_set_offset(0, 0);
    terminal_set_size((uint64_t)gfx.width(), (uint64_t)gfx.height());
    fb_clear(0x0D0D0D);
    terminal_putchar('\f');
    kprintf("luna: exited\n");
}
