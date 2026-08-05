#include "button.hpp"
#include "../focus.hpp"

void Button::paint(Gfx &g)
{
    int sx, sy;
    screen_pos(sx, sy);

    uint32_t face = pressed ? 0xFFB0B0B0u : 0xFFD0D0D0u;
    g.fill_rect(sx, sy, w, h, face);
    g.draw_rect(sx, sy, w, h, 0xFF404040u);

    if (text)
    {
        int tx = sx + 8;
        int ty = sy + (h / 2) - 4;
        g.draw_text(tx, ty, text, 0xFF000000u, face);
    }
}

bool Button::on_event(const Event &e)
{
    /* children first (none usually) */
    if (Widget::on_event(e))
        return true;

    if (!contains_screen(e.x, e.y))
    {
        if (e.type == EventType::MouseUp && pressed)
        {
            pressed = false;
            mark_dirty();
        }
        return false;
    }

    if (e.type == EventType::MouseDown && (e.buttons & 1))
    {
        Focus::clear();
        pressed = true;
        mark_dirty();
        return true;
    }

    if (e.type == EventType::MouseUp)
    {
        bool was = pressed;
        pressed = false;
        mark_dirty();
        if (was && on_click)
            on_click(on_click_user);
        return true;
    }

    return false;
}
