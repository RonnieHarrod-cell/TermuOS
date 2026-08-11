#include "../widgets/widget.hpp"
#include "startmenu/startmenu.hpp"
#include "wm.hpp"

class Taskbar : public Widget
{
public:
    Wm *wm = nullptr;
    static constexpr int kH = 36;

    void layout(int screen_w, int screen_h)
    {
        x = 0;
        y = screen_h - kH;
        w = screen_w;
        h = kH;
        visible = true;
        parent = nullptr;
    }

    StartMenu *menu = nullptr;

    void paint(Gfx &g)
    {
        int sx = x, sy = y;
        if (parent)
            screen_pos(sx, sy);

        g.fill_rect(sx, sy, w, h, 0xFF121826u);
        g.fill_rect(sx, sy, w, 1, 0xFF3A455Fu);
        g.fill_rect(sx, sy + h - 1, w, 1, 0xFF0A0E18u);

        // start button
        int bx = sx + 6, by = sy + 4, bw = 72, bh = h - 8;
        g.fill_rect(bx, by, bw, bh, 0xFF5B8CFFu);
        g.fill_rect(bx, by, bw, 1, 0xFF8BB0FFu);
        g.draw_text(bx + 14, by + (bh - 16) / 2, "Start", 0xFFFFFFFFu, 0xFF5B8CFFu);

        g.draw_text(sx + 90, sy + 10, "Luna", 0xFF8B93A7u, 0xFF121826u);

        g.draw_text(sx + w - 60, sy + 10, "12:00", 0xFFC5CAD6u, 0xFF121826u);
    }

    bool on_event(const Event &e) override
    {
        if (!contains_screen(e.x, e.y))
            return false;

        if (e.type == EventType::MouseDown)
        {
            int sx, sy;
            screen_pos(sx, sy);
            if (e.x < sx + 80)
            {
                if (menu)
                    menu->toggle(y + h);
                mark_dirty();
                return true;
            }
        }
        return true;
    }
};
