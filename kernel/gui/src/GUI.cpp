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
}
