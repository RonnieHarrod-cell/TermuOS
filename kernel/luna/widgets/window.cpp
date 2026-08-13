#include "window.hpp"
#include "../focus.hpp"
#include "../theme.hpp"

void Window::paint(Gfx &g)
{
    int sx, sy;
    screen_pos(sx, sy);

    g.draw_raised(sx, sy, w, h);

    /* title bar */
    int tx = sx + 3, ty = sy + 3, tw = w - 6, th = kTitleH;

    for (int i = 0; i < tw; i++)
    {
        int t = (tw > 1) ? i : 0;
        int r = 0 + (0x10 * t) / (tw > 1 ? tw - 1 : 1);
        int gr = 0 + (0x84 * t) / (tw > 1 ? tw - 1 : 1);
        int b = 0x80 + ((0xD0 - 0x80) * t) / (tw > 1 ? tw - 1 : 1);
        uint32_t col = 0xFF000000u | ((uint32_t)r << 16) | ((uint32_t)gr << 8) | (uint32_t)b;
        g.fill_rect(tx + i, ty, 1, th - 1, col);
    }

    g.draw_text(tx + 4, ty + 2, title ? title : "", Theme::title_text, Theme::title_left);

    /* close button */
    g.draw_raised(sx + w - 20, sy + 5, 16, 14);
    g.draw_text(sx + w - 16, sy + 4, "x", Theme::text, Theme::face);

    /* client area */
    g.fill_rect(sx + 3, sy + 3 + kTitleH, w - 6, h - 6 - kTitleH, Theme::client);
}

bool Window::on_event(const Event &e)
{
    int sx, sy;
    screen_pos(sx, sy);

    /* close button */
    int cx0 = sx + w - kBorder - 16;
    int cy0 = sy + kBorder + 4;
    if (e.type == EventType::MouseDown && (e.buttons & 1) &&
        e.x >= cx0 && e.x < cx0 + 12 && e.y >= cy0 && e.y < cy0 + 12)
    {
        visible = false;
        mark_dirty();
        return true;
    }

    /* title drag */
    if (e.type == EventType::MouseDown && (e.buttons & 1) &&
        e.y >= sy + kBorder && e.y < sy + kBorder + kTitleH &&
        e.x >= sx + kBorder && e.x < sx + w - kBorder - 20)
    {
        dragging_ = true;
        drag_ox_ = e.x - sx;
        drag_oy_ = e.y - sy;
        return true;
    }

    if (Widget::on_event(e))
        return true;

    if (e.type == EventType::MouseDown && contains_screen(e.x, e.y))
    {
        Focus::clear();
        mark_dirty();
        return true;
    }

    if (e.type == EventType::MouseUp)
        dragging_ = false;

    if (e.type == EventType::MouseMove && dragging_)
    {
        x = e.x - drag_ox_;
        y = e.y - drag_oy_;
        if (parent)
        {
            /* clamp later if you want */
        }
        mark_dirty();
        return true;
    }

    return Widget::on_event(e);
}
