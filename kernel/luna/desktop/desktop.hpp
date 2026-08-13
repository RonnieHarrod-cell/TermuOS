#pragma once
#include "../widgets/widget.hpp"
#include "wm.hpp"
#include "../focus.hpp"
#include "../theme.hpp"

class Taskbar;

class Desktop : public Widget
{
public:
    uint32_t wallpaper = 0xFF1A1A2Eu;
    Wm *wm = nullptr;
    Taskbar *taskbar = nullptr;

    static uint32_t lerp_col(uint32_t a, uint32_t b, int t, int n)
    {
        // t in [0, n]
        auto ch = [&](int shift)
        {
            int ca = (a >> shift) & 0xFF;
            int cb = (b >> shift) & 0xFF;
            return (uint32_t)(ca + (cb - ca) * t / n);
        };
        return 0xFF000000u | (ch(16) << 16) | (ch(8) << 8) | ch(0);
    }

    void paint(Gfx &g) override
    {
        g.fill_rect(0, 0, w, h, Theme::desktop);
    }

    bool on_event(const Event &e) override
    {
        if (Widget::on_event(e))
            return true;
        if (e.type == EventType::MouseDown)
        {
            Focus::clear();
            mark_dirty();
            return true;
        }
        return false;
    }
};
