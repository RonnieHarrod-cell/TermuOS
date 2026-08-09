#include "luna.h"
#include "widgets/gfx.hpp"
#include "widgets/window.hpp"
#include "widgets/label.hpp"
#include "widgets/button.hpp"
#include "widgets/textfield.hpp"
#include "widgets/events.hpp"
#include "focus.hpp"
#include "desktop/desktop.hpp"
#include "desktop/wm.hpp"
#include "desktop/taskbar.hpp"

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
            g.put_pixel(g_omx + col, g_omy + row,
                        g_cursor_under[row * CURSOR_W + col]);
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
            g_cursor_under[row * CURSOR_W + col] =
                g.get_pixel(g_mx + col, g_my + row);
    g_cursor_saved = 1;
    g_omx = g_mx;
    g_omy = g_my;

    for (int r = 0; r < 15; r++)
        for (int c = 0; c < 11; c++)
        {
            char ch = shape[r][c];
            if (ch == ' ')
                continue;
            g.put_pixel(g_mx + c, g_my + r,
                        ch == 'X' ? 0xFFFFFFu : 0x000000u);
        }
}

static void on_btn(void *user)
{
    (void)user;
    kprintf("luna: button clicked\n");
}

static bool *g_running_ptr = nullptr;
static Wm *g_wm = nullptr;
static Window *g_about = nullptr;

static void action_exit(void *user)
{
    (void)user;
    if (g_running_ptr)
        *g_running_ptr = false;
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

    /* —— desktop shell —— */
    Desktop desk;
    desk.x = 0;
    desk.y = 0;
    desk.w = gfx.width();
    desk.h = gfx.height();

    Wm wm;
    desk.wm = &wm;

    Taskbar bar;
    bar.wm = &wm;
    bar.layout(gfx.width(), gfx.height());
    desk.taskbar = &bar;

    StartMenu menu;
    menu.visible = false;
    menu.open = false;
    menu.parent = nullptr;
    menu.add_item("Exit Luna", action_exit, nullptr);

    bar.menu = &menu;

    /* —— demo window + widgets —— */
    Window win;
    win.x = 120;
    win.y = 80;
    win.w = 420;
    win.h = 300;
    win.title = "Demo - Luna";
    win.visible = true;

    Label lab;
    lab.x = Window::kBorder + 8;
    lab.y = Window::kBorder + Window::kTitleH + 8;
    lab.w = 300;
    lab.h = 20;
    lab.text = "Luna desktop";
    win.add(&lab);

    Button btn;
    btn.x = Window::kBorder + 8;
    btn.y = Window::kBorder + Window::kTitleH + 40;
    btn.w = 120;
    btn.h = 28;
    btn.text = "Click me";
    btn.on_click = on_btn;
    win.add(&btn);

    TextField field;
    field.x = Window::kBorder + 8;
    field.y = Window::kBorder + Window::kTitleH + 80;
    field.w = 280;
    field.h = 28;
    win.add(&field);

    wm.add(&win);

    auto composite = [&]()
    {
        g_cursor_saved = 0;
        desk.paint_tree(gfx);
        wm.paint_all(gfx);
        bar.paint_tree(gfx);
        if (menu.open && menu.visible)
            menu.paint_tree(gfx);
        cursor_draw(gfx);
    };

    composite();
    kprintf("luna: desktop running — Esc to quit\n");

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
                full = true;
            }
            else
            {
                ev.type = EventType::MouseMove;
            }

            /* ---- hit test (one path only) ---- */
            if (menu.open && menu.visible && menu.contains_screen(ev.x, ev.y))
            {
                menu.on_event(ev);
                full = true;
            }
            else if (bar.contains_screen(ev.x, ev.y))
            {
                bar.on_event(ev); /* Start click → menu.toggle(height) */
                full = true;
            }
            else if (Window *hit = wm.hit(ev.x, ev.y))
            {
                if (menu.open)
                    menu.close_menu();
                if (ev.type == EventType::MouseDown)
                    wm.raise(hit);
                hit->on_event(ev);
            }
            else
            {
                if (menu.open && ev.type == EventType::MouseDown)
                    menu.close_menu();
                desk.on_event(ev);
                if (ev.type == EventType::MouseDown)
                    full = true;
            }

            if (win.dirty || desk.dirty || bar.dirty || menu.dirty)
                full = true;

            if (full)
            {
                win.dirty = desk.dirty = bar.dirty = menu.dirty = false;
                g_mx = st.x;
                g_my = st.y;
                composite();
            }
            else
            {
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
                if (Focus::current())
                {
                    Focus::clear();
                    composite();
                }
                else
                {
                    g_running = false;
                }
                continue;
            }

            Event kev{};
            kev.type = EventType::KeyDown;
            kev.key = c;
            kev.x = g_mx;
            kev.y = g_my;

            if (Widget *f = Focus::current())
            {
                f->on_event(kev);
                composite();
            }
        }
    }

    terminal_set_offset(0, 0);
    terminal_set_size((uint64_t)gfx.width(), (uint64_t)gfx.height());
    fb_clear(0x0D0D0D);
    terminal_putchar('\f');
    kprintf("luna: exited\n");
}
