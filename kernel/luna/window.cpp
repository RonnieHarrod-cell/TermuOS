#include "window.hpp"
#include "focus.hpp"

void Window::paint(Gfx &g)
{
    int sx, sy;
    screen_pos(sx, sy);

    g.fill_rect(sx, sy, w, h, 0xFF3A3A4Au);
    g.fill_rect(sx + kBorder, sy + kBorder, w - kBorder * 2, kTitleH, 0xFF2D5B8Au);
    g.fill_rect(sx + w - kBorder - 16, sy + kBorder + 4, 12, 12, 0xFFCC3333u);
    g.fill_rect(sx + kBorder, sy + kBorder + kTitleH,
                w - kBorder * 2, h - kTitleH - kBorder * 2, 0xFFE8E8EEu);

    if (title)
        g.draw_text(sx + kBorder + 6, sy + kBorder + 6, title, 0xFFFFFFFFu, 0xFF2D5B8Au);
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
