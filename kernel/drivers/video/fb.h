#pragma once
#include <stdint.h>
#include <limine.h>

#ifdef __cplusplus
extern "C"
{
#endif

    struct limine_framebuffer *fb_get(void);

    void fb_init(struct limine_framebuffer *fb);
    void fb_clear(uint32_t colour);
    uint32_t fb_colour(uint8_t r, uint8_t g, uint8_t b);
    void fb_putpixel(uint64_t x, uint64_t y, uint32_t colour);

    void fb_fill_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t colour);
    void fb_draw_rect(uint64_t x, uint64_t y, uint64_t w, uint64_t h, uint32_t colour); // outline
    void fb_draw_hline(uint64_t x, uint64_t y, uint64_t len, uint32_t colour);
    void fb_draw_vline(uint64_t x, uint64_t y, uint64_t len, uint32_t colour);

    uint32_t fb_getpixel(uint64_t x, uint64_t y);

#ifdef __cplusplus
}
#endif
