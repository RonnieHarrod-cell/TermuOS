#pragma once
#include "../widgets/widget.hpp"
#include "../icon.h"

class DeskIcon : public Widget
{
public:
    const char *label = "";
    const uint8_t *rgba = nullptr;
    int iw = 32, ih = 32;
    void (*on_open)(void *user) = nullptr;
    void *user = nullptr;

    void paint(Gfx &g) override
    {
        int sx, sy;
        screen_pos(sx, sy);
        if (rgba)
            g.blit_rgba(sx + (w - iw) / 2, sy, iw, ih, rgba);
        if (label)
            g.draw_text(sx + 2, sy + ih + 4, label, 0xFFE8ECF4u, 0xFF0B1020u);
    }

    bool on_event(const Event &e) override
    {
        if (e.type == EventType::MouseDown && contains_screen(e.x, e.y))
        {
            if (on_open)
                on_open(user);
            mark_dirty();
            return true;
        }
        return false;
    }
};
