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
#include "desktop/deskicon.hpp"
#include "apps/app.hpp"

extern "C"
{
#include "../drivers/input/mouse.h"
#include "../drivers/input/keyboard.h"
#include "../drivers/video/fb.h"
#include "../drivers/video/terminal.h"
#include "../drivers/rtc/rtc.h"
#include "../arch/x86_64/pit.h"
#include "../lib/printf.h"
#include "icon.h"
}

static constexpr int CURSOR_W = 12;
static constexpr int CURSOR_H = 18;

static int g_mx, g_my, g_omx, g_omy;
static int g_cursor_saved;
static uint32_t g_cursor_under[CURSOR_W * CURSOR_H];
static bool g_running;
static bool *g_running_ptr = nullptr;

static Desktop *g_desk = nullptr;

void luna_set_wallpaper(uint32_t colour)
{
    if (!g_desk)
        return;
    g_desk->wallpaper = colour;
    g_desk->mark_dirty();
}

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
                        ch == 'X' ? 0xFFFFFFFFu : 0x000000u);
        }
}

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
    g_running_ptr = &g_running;

    Desktop desk;
    desk.x = 0;
    desk.y = 0;
    desk.w = gfx.width();
    desk.h = gfx.height();
    g_desk = &desk;
    desk.wallpaper = Theme::desktop;

    Wm wm;
    desk.wm = &wm;
    g_luna_wm = &wm;

    Taskbar bar;
    bar.wm = &wm;
    bar.layout(gfx.width(), gfx.height());
    desk.taskbar = &bar;

    StartMenu menu;
    menu.visible = false;
    menu.open = false;
    menu.parent = nullptr;

    luna_apps_register_menu(menu);
    menu.add_item("Exit Luna", "System", action_exit, nullptr);
    bar.menu = &menu;

    luna_icon_t about_ic{};
    luna_icon_load("about.rgba", 32, 32, &about_ic);

    DeskIcon icon_about;
    icon_about.x = 24;
    icon_about.y = 24;
    icon_about.w = 72;
    icon_about.h = 60;
    icon_about.label = "About";
    icon_about.rgba = about_ic.rgba;
    icon_about.iw = about_ic.w;
    icon_about.ih = about_ic.h;
    icon_about.on_open = app_about_open;
    icon_about.user = nullptr;

    desk.add(&icon_about);

    auto composite = [&]()
    {
        g_cursor_saved = 0;
        desk.paint_tree(gfx);
        wm.paint_all(gfx);

        app_terminal_paint();

        bar.paint_tree(gfx);
        if (menu.open && menu.visible)
            menu.paint_tree(gfx);
        cursor_draw(gfx);
    };

    composite();
    kprintf("luna: desktop running — Esc to quit\n");

    uint8_t prev_buttons = 0;
    Window *drag_win = nullptr;

    static int last_min = -1;

    while (g_running)
    {
        rtc_time_t t;
        rtc_read(&t);
        if ((int)t.minute != last_min)
        {
            last_min = t.minute;
            bar.mark_dirty();
        }

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

            if (drag_win && (st.buttons & 1) &&
                (ev.type == EventType::MouseMove || ev.type == EventType::MouseUp))
            {
                drag_win->on_event(ev);
                full = true;
                if (ev.type == EventType::MouseUp)
                    drag_win = nullptr;
            }
            else if (menu.open && menu.visible &&
                     menu.contains_screen(ev.x, ev.y))
            {
                menu.on_event(ev);
                full = true;
            }
            else if (bar.contains_screen(ev.x, ev.y))
            {
                bar.on_event(ev);
                full = true;
            }
            else if (Window *hit = wm.hit(ev.x, ev.y))
            {
                if (menu.open)
                    menu.close_menu();
                if (ev.type == EventType::MouseDown)
                {
                    wm.raise(hit);
                    drag_win = hit;
                }
                hit->on_event(ev);
                if (ev.type == EventType::MouseUp)
                    drag_win = nullptr;
            }
            else
            {
                if (menu.open && ev.type == EventType::MouseDown)
                    menu.close_menu();
                desk.on_event(ev);
                if (ev.type == EventType::MouseDown)
                {
                    drag_win = nullptr;
                    full = true;
                } f
            }

            if (desk.dirty || bar.dirty || menu.dirty)
                full = true;

            for (int i = 0; i < wm.count; i++)
            {
                if (wm.stack[i] && wm.stack[i]->dirty)
                    full = true;
            }

            if (app_explorer_is_open())
                app_explorer_click(ev.x, ev.y);

            if (full)
            {
                desk.dirty = bar.dirty = menu.dirty = false;
                for (int i = 0; i < wm.count; i++)
                    if (wm.stack[i])
                        wm.stack[i]->dirty = false;

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

            if (app_terminal_is_open())
            {
                app_terminal_handle_key(c);
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

    g_luna_wm = nullptr;
    g_running_ptr = nullptr;

    terminal_set_offset(0, 0);
    terminal_set_size((uint64_t)gfx.width(), (uint64_t)gfx.height());
    fb_clear(0x0D0D0D);
    terminal_putchar('\f');
    kprintf("luna: exited\n");
}
