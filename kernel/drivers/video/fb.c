#include "fb.h"

static struct limine_framebuffer *_fb = 0;

void fb_init(struct limine_framebuffer *fb)
{
    _fb = fb;
}

struct limine_framebuffer *fb_get(void)
{
    return _fb;
}

uint32_t fb_colour(uint8_t r, uint8_t g, uint8_t b)
{
    return ((uint32_t)r << _fb->red_mask_shift) | ((uint32_t)g << _fb->green_mask_shift) | ((uint32_t)b << _fb->blue_mask_shift);
}

void fb_putpixel(uint64_t x, uint64_t y, uint32_t colour)
{
    if (!_fb || x >= _fb->width || y >= _fb->height)
        return;
    uint8_t *base = (uint8_t *)_fb->address;
    uint32_t *row = (uint32_t *)(base + y * _fb->pitch);
    row[x] = colour;
}

void fb_clear(uint32_t colour)
{
    if (!_fb)
        return;
    uint8_t *base = (uint8_t *)_fb->address;
    for (uint64_t y = 0; y < _fb->height; y++)
    {
        uint32_t *row = (uint32_t *)(base + y * _fb->pitch);
        for (uint64_t x = 0; x < _fb->width; x++)
            row[x] = colour;
    }
}

void fb_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t colour) 
{
    if (!_fb)
        return;
    if (x >= _fb->width || y >= _fb->height)
        return;
    if (x + w > _fb->width)
        w = _fb->width - x;
    if (y + h > _fb->height)
        h = _fb->height - y;

    uint8_t *base = (uint8_t *)_fb->address;
    for (uint64_t row = 0; row < h; row++)
    {
        uint32_t *line = (uint32_t *)(base + (y + row) * _fb->pitch);
        for (uint64_t col = 0; col < w; col++)
            line[x + col] = colour;
    }
}

void fb_draw_hline(uint64_t x, uint64_t y, uint64_t len, uint32_t colour)
{
    fb_fill_rect(x, y, len, 1, colour);
}

void fb_draw_vline(uint64_t x, uint64_t y, uint64_t len, uint32_t colour)
{
    fb_fill_rect(x, y, 1, len, colour);
}

void fb_draw_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t colour)
{
    if (w == 0 || h == 0)
        return;
    fb_draw_hline(x, y, w, colour);
    fb_draw_hline(x, y + h - 1, w, colour);
    fb_draw_vline(x, y, h, colour);
    fb_draw_vline(x + w - 1, y, h, colour);
}

uint32_t fb_getpixel(uint64_t x, uint64_t y)
{
    if (!_fb || x >= _fb->width || y >= _fb->height)
        return 0;
    uint8_t *base = (uint8_t *)_fb->address;
    uint32_t *row = (uint32_t *)(base + y * _fb->pitch);
    return row[x];
}
