#pragma once
#include "../widgets/widget.hpp"
#include "wm.hpp"
#include "../focus.hpp"

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
        uint32_t top = 0xFF0B1020u;
        uint32_t bot = 0xFF1A2744u;
        for (int y = 0; y < h; y++)
            g.fill_rect(0, y, w, 1, lerp_col(top, bot, y, h > 1 ? h - 1 : 1));

        int cx = w / 3, cy = h / 4, r = 120;
        for (int dy = -r; dy <= r; dy++)
        {
            for (int dx = -r; dx <= r; dx++)
            {
                int d2 = dx * dx + dy * dy;
                if (d2 > r * r)
                    continue;
                int x = cx + dx, y = cy + dy;
                if (x < 0 | y < 0 || x >= w || y >= h)
                    continue;
                // faint blue wash
                if ((d2 & 15) == 0)
                    g.put_pixel(x, y, 0xFF1A2A50u);
            }
        }
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
