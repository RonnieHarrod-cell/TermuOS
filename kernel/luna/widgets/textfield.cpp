#include "textfield.hpp"
#include "../focus.hpp"

void TextField::on_focus()
{
    focused_ = true;
    mark_dirty();
}

void TextField::on_blur()
{
    focused_ = false;
    mark_dirty();
}

void TextField::paint(Gfx &g)
{
    int sx, sy;
    screen_pos(sx, sy);

    g.fill_rect(sx, sy, w, h, bg);
    g.draw_rect(sx, sy, w, h, focused_ ? border_focus : border);

    buffer[length] = '\0';
    g.draw_text(sx + 4, sy + (h - 16) / 2, buffer, fg, bg);

    if (focused_)
    {
        /* caret after `caret` characters (8px font width) */
        int cx = sx + 4 + caret * 8;
        int cy = sy + 4;
        g.fill_rect(cx, cy, 2, h - 8, fg);
    }
}

bool TextField::on_event(const Event &e)
{
    if (Widget::on_event(e))
        return true;

    if (e.type == EventType::MouseDown && contains_screen(e.x, e.y))
    {
        Focus::set(this);
        /* optional: set caret from click x */
        int sx, sy;
        screen_pos(sx, sy);
        int rel = (e.x - sx - 4) / 8;
        if (rel < 0)
            rel = 0;
        if (rel > length)
            rel = length;
        caret = rel;
        mark_dirty();
        return true;
    }

    if (!focused_)
        return false;

    if (e.type != EventType::KeyDown)
        return false;

    char c = e.key;

    if (c == '\b' || c == 127)
    { /* backspace */
        if (caret > 0)
        {
            for (int i = caret - 1; i < length; i++)
                buffer[i] = buffer[i + 1];
            length--;
            caret--;
            mark_dirty();
        }
        return true;
    }

    if (c == '\n' || c == '\r')
    {
        /* Enter: leave for app callback later */
        return true;
    }

    if (c >= 32 && c < 127 && length < kCap - 1)
    {
        for (int i = length; i > caret; i--)
            buffer[i] = buffer[i - 1];
        buffer[caret] = c;
        length++;
        caret++;
        buffer[length] = '\0';
        mark_dirty();
        return true;
    }

    return true; /* ate the key while focused */
}
