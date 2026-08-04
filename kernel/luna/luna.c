#include "luna.h"
#include "../drivers/video/fb.h"
#include "../drivers/video/terminal.h"
#include "../drivers/input/mouse.h"
#include "../drivers/input/keyboard.h"
#include "../lib/printf.h"
#include <stdint.h>

#define TITLE_H 24
#define BORDER 3
#define CURSOR_W 12
#define CURSOR_H 18

static int scr_w, scr_h;
static int mx, my, omx, omy;
static int running;
static int cursor_saved;
static uint32_t cursor_under[CURSOR_W * CURSOR_H];

static void put(int x, int y, uint32_t c)
{
    if (x < 0 || y < 0 || x >= scr_w || y >= scr_h)
        return;
    fb_putpixel((uint64_t)x, (uint64_t)y, c);
}

static void fill(int x, int y, int w, int h, uint32_t c)
{
    if (w <= 0 || h <= 0)
        return;
    fb_fill_rect((uint64_t)x, (uint64_t)y, (uint64_t)w, (uint64_t)h, c);
}

static void erase_cursor(void)
{
    if (!cursor_saved)
        return;
    for (int row = 0; row < CURSOR_H; row++)
        for (int col = 0; col < CURSOR_W; col++)
        {
            int x = omx + col, y = omy + row;
            if (x >= 0 && y >= 0 && x < scr_w && y < scr_h)
                put(x, y, cursor_under[row * CURSOR_W + col]);
        }
    cursor_saved = 0;
}

static void draw_cursor(void)
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

    if (mx < 0) mx = 0;
    if (my < 0) my = 0;
    if (mx > scr_w - CURSOR_W) mx = scr_w - CURSOR_W;
    if (my > scr_h - CURSOR_H) my = scr_h - CURSOR_H;
    if (mx < 0) mx = 0;
    if (my < 0) my = 0;

    for (int row = 0; row < CURSOR_H; row++)
        for (int col = 0; col < CURSOR_W; col++)
        {
            int x = mx + col, y = my + row;
            uint32_t pix = 0;
            if (x >= 0 && y >= 0 && x < scr_w && y < scr_h)
                pix = fb_getpixel((uint64_t)x, (uint64_t)y);
            cursor_under[row * CURSOR_W + col] = pix;
        }
    cursor_saved = 1;
    omx = mx;
    omy = my;

    for (int r = 0; r < 15; r++)
    {
        for (int c = 0; c < 11; c++)
        {
            char ch = shape[r][c];
            if (ch == ' ')
                continue;
            put(mx + c, my + r, ch == 'X' ? 0x000000u : 0xFFFFFFu);
        }
    }
}

static void draw_window(int x, int y, int w, int h, const char *title)
{
    fill(x, y, w, h, 0x3A3A4Au);
    fill(x + BORDER, y + BORDER, w - BORDER * 2, TITLE_H, 0x2D5B8Au);
    fill(x + w - BORDER - 16, y + BORDER + 4, 12, 12, 0xCC3333u);
    fill(x + BORDER, y + BORDER + TITLE_H,
         w - BORDER * 2, h - TITLE_H - BORDER * 2, 0xE8E8EEu);
    (void)title;
}

static void composite(void)
{
    cursor_saved = 0;
    fill(0, 0, scr_w, scr_h, 0x1A1A2Eu);
    draw_window(120, 80, 480, 320, "Luna");
    draw_cursor();
}

void luna_run(void)
{
    struct limine_framebuffer *fb = fb_get();
    if (!fb)
        return;

    scr_w = (int)fb->width;
    scr_h = (int)fb->height;
    mouse_set_bounds(scr_w, scr_h);
    mx = scr_w / 2;
    my = scr_h / 2;
    running = 1;

    composite();
    kprintf("luna: running - move mouse, Esc to quit\n");

    while (running)
    {
        if (mouse_is_dirty())
        {
            mouse_state_t st;
            mouse_get_state(&st);
            erase_cursor();
            mx = st.x;
            my = st.y;
            draw_cursor();
        }

        if (keyboard_haschar())
        {
            char c = keyboard_getchar();
            if (c == 27) // Esc
                running = 0;
        }
    }

    terminal_set_offset(0, 0);
    terminal_set_size((uint64_t)scr_w, (uint64_t)scr_h);
    fb_clear(0x0D0D0D);
    terminal_putchar('\f');
    kprintf("luna: exited\n");
}
