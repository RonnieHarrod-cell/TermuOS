#pragma once

#include "Graphics.hpp"
#include "Window.hpp"
#include <limine.h>

class GUI
{
public:
    void init(limine_framebuffer *fb);
    void update();

    bool add_window(Window *w);
    void set_background(uint8_t r, uint8_t g, uint8_t b);

private:
    Graphics gfx;
    static constexpr int MAX_WINDOWS = 16;
    Window *windows_[MAX_WINDOWS] = {};
    int window_count_ = 0;

    uint32_t bg_colour_ = 0;
};
