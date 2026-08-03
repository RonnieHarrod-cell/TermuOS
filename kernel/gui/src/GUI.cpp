#include "../include/GUI.hpp"
#include "../../drivers/video/fb.h"

void GUI::init(limine_framebuffer *framebuffer)
{
    gfx.init(framebuffer);
    bg_colour_ = gfx.colour(0x0D, 0x0D, 0x18);
    window_count_ = 0;
}

void GUI::set_background(uint8_t r, uint8_t g, uint8_t b)
{
    bg_colour_ = gfx.colour(r, g, b);
}

bool GUI::add_window(Window *w)
{
    if (!w || window_count_ >= MAX_WINDOWS)
        return false;
    windows_[window_count_++] = w;
    return true;
}

void GUI::set_mouse(int x, int y)
{
    mouse_x_ = x;
    mouse_y_ = y;
}

void GUI::draw_cursor()
{
    if (!mouse_visible_)
        return;

    // Simple arrow cursor (11x16-ish)
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
        "X........X ",
        "X.....XXXXX",
        "X..X..X    ",
        "X.X X..X   ",
        "XX  X..X   ",
        "X    X..X  ",
        "     X..X  ",
        "      XX   ",
    };
    const int rows = 17;
    const int cols = 11;

    uint32_t white = gfx.colour(0xFF, 0xFF, 0xFF);
    uint32_t black = gfx.colour(0x00, 0x00, 0x00);

    for (int row = 0; row < rows; row++)
    {
        for (int col = 0; col < cols; col++)
        {
            char c = shape[row][col];
            if (c == ' ')
                continue;
            uint32_t colr = (c == 'X') ? black : white;
            gfx.put_pixel(mouse_x_ + col, mouse_y_ + row, colr);
        }
    }
}

void GUI::update()
{
    // Clear desktop
    gfx.clear(bg_colour_);

    // Draw all visible windows
    for (int i = 0; i < window_count_; i++)
    {
        if (windows_[i] && windows_[i]->visible)
            windows_[i]->draw(gfx);
    }

    draw_cursor();
}
