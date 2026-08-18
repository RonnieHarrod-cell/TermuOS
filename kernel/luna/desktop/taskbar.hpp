#include "../widgets/widget.hpp"
#include "startmenu/startmenu.hpp"
#include "wm.hpp"
#include "../theme.hpp"

extern "C"
{
#include "../../drivers/rtc/rtc.h"
}

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

        g.draw_raised(sx, sy, w, h);
        /* top highlight already in raised; optional extra line */
        g.fill_rect(sx, sy, w, 1, Theme::highlight);

        /* Start button — raised, pressed = sunken when menu open */
        int bx = sx + 2, by = sy + 3, bw = 54, bh = h - 6;
        if (menu && menu->open)
            g.draw_sunken(bx, by, bw, bh);
        else
            g.draw_raised(bx, by, bw, bh);
        g.draw_text(bx + 8, by + 4, "Start", Theme::text, Theme::face);

        /* clock tray (right) */
        rtc_time_t t;
        rtc_read(&t);

        char clock[16];
        /* HH:MM */
        clock[0] = '0' + (t.hour / 10);
        clock[1] = '0' + (t.hour % 10);
        clock[2] = ':';
        clock[3] = '0' + (t.minute / 10);
        clock[4] = '0' + (t.minute % 10);
        clock[5] = '\0';

        int cw = 54;
        int ch = h - 6;
        int cx = sx + w - cw - 4;
        int cy = sy + 3;

        g.draw_sunken(cx, cy, cw, ch);
        g.draw_text(cx + 8, cy + 4, clock, Theme::text, Theme::face);
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
