#pragma once
#include <stdint.h>

class Gfx
{
public:
    void init_from_fb();
    int width() const;
    int height() const;
    void fill_rect(int x, int y, int w, int h, uint32_t colour);
    void draw_rect(int x, int y, int w, int h, uint32_t colour);
    void put_pixel(int x, int y, uint32_t colour);
    uint32_t get_pixel(int x, int y);
    void draw_text(int x, int y, const char *s, uint32_t fg, uint32_t bg);
    static uint32_t rgb(uint8_t r, uint8_t g, uint8_t b);

    void blit_rgba(int x, int y, int w, int h, const uint8_t *rgba);

    void draw_raised(int x, int y, int w, int h);
    void draw_sunken(int x, int y, int w, int h);

private:
    int w_ = 0, h_ = 0;
    void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
};
