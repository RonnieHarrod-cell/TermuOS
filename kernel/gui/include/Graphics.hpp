#pragma once

#include <stdint.h>
#include <limine.h>

class Graphics 
{
public:
    void init(limine_framebuffer *fb);

    void clear(uint32_t colour);
    void fill_rect(int x, int y, int w, int h, uint32_t colour);
    void draw_rect(int x, int y, int w, int h, uint32_t colour);
    void put_pixel(int x, int y, uint32_t colour);

    void draw_char(int x, int y, char c, uint32_t fg, uint32_t bg);
    void draw_string(int x, int y, const char *str, uint32_t fg, uint32_t bg);

    uint32_t colour(uint8_t r, uint8_t g, uint8_t b);

    int width() const;
    int height() const;

    static constexpr int FONT_W = 8;
    static constexpr int FONT_H = 16;

private:
    limine_framebuffer *fb_ = nullptr;
};
