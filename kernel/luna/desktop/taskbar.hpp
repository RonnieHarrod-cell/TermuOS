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
    }

    StartMenu *menu = nullptr;

    void paint(Gfx &g) override
    {
        int sx, sy;
        screen_pos(sx, sy);
        g.fill_rect(sx + 4, sy + 4, 72, h - 8, 0xFF3A3A5Au);
        g.draw_text(sx + 12, sy + 10, "Start", 0xFFFFFFFFu, 0xFF3A3A5Au);
        /* later: window buttons + clock */
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
