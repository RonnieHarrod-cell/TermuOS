#include "widget.hpp"

class Label : public Widget
{
public:
    const char *text = "";
    uint32_t fg = 0xFF000000;
    uint32_t bg = 0xFFE8E8EE;

    void paint(Gfx &g) override
    {
        int sx, sy;
        screen_pos(sx, sy);
        g.fill_rect(sx, sy, w, h, bg);
        g.draw_text(sx + 4, sy + 4, text, fg, bg);
    }
};
