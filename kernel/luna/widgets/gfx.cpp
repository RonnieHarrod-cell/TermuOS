#include "gfx.hpp"
#include "../../drivers/video/fb.h"
#include "../../lib/font.h"

void Gfx::init_from_fb()
{
    struct limine_framebuffer *fb = fb_get();
    if (!fb)
    {
        w_ = h_ = 0;
        return;
    }
    w_ = (int)fb->width;
    h_ = (int)fb->height;
}

int Gfx::width() const { return w_; }
int Gfx::height() const { return h_; }

void Gfx::fill_rect(int x, int y, int w, int h, uint32_t colour)
{
    if (w <= 0 || h <= 0)
        return;
    fb_fill_rect((uint64_t)x, (uint64_t)y, (uint64_t)w, (uint64_t)h, colour);
}

void Gfx::draw_rect(int x, int y, int w, int h, uint32_t colour)
{
    if (w <= 0 || h <= 0)
        return;
    fb_draw_rect((uint64_t)x, (uint64_t)y, (uint64_t)w, (uint64_t)h, colour);
}

void Gfx::put_pixel(int x, int y, uint32_t colour)
{
    if (x < 0 || y < 0 || x >= w_ || y >= h_)
        return;
    fb_putpixel((uint64_t)x, (uint64_t)y, colour);
}

uint32_t Gfx::get_pixel(int x, int y)
{
    if (x < 0 || y < 0 || x >= w_ || y >= h_)
        return 0;
    return fb_getpixel((uint64_t)x, (uint64_t)y);
}

uint32_t Gfx::rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return fb_colour(r, g, b);
}

void Gfx::draw_char(int x, int y, char c, uint32_t fg, uint32_t bg)
{
    if (c < 32 || c > 126)
        c = '?';
    const uint8_t *glyph = g_font_8x16[(unsigned char)c - 32];
    for (int row = 0; row < FONT_H; row++)
    {
        uint8_t bits = glyph[row];
        for (int col = 0; col < FONT_W; col++)
        {
            uint32_t colr = (bits & (0x80 >> col)) ? fg : bg;
            put_pixel(x + col, y + row, colr);
        }
    }
}

void Gfx::draw_text(int x, int y, const char *s, uint32_t fg, uint32_t bg)
{
    if (!s)
        return;
    int cx = x;
    for (; *s; ++s)
    {
        if (*s == '\n')
        {
            cx = x;
            y += FONT_H;
            continue;
        }
        draw_char(cx, y, *s, fg, bg);
        cx += FONT_W;
    }
}
