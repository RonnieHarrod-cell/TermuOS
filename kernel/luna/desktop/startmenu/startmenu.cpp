#include "startmenu.hpp"
#include "../../focus.hpp"

void StartMenu::add_item(const char *label, void (*action)(void *), void *user)
{
    if (item_count >= kMaxItems)
        return;
    items[item_count].label = label;
    items[item_count].action = action;
    items[item_count].user = user;
    item_count++;
}

void StartMenu::close_menu()
{
    open = false;
    visible = false;
    mark_dirty();
}

void StartMenu::toggle(int screen_h)
{
    open = !open;
    visible = open;
    if (open)
    {
        h = kPad * 2 + item_count * kItemH;
        w = 160;
        x = 4;
        y = screen_h - 36 - h - 4;
    }
    mark_dirty();
}

void StartMenu::paint(Gfx &g)
{
    if (!open || !visible)
        return;

    int sx, sy;
    screen_pos(sx, sy);

    g.fill_rect(sx, sy, w, h, 0xFF2A2A3Au);
    g.draw_rect(sx, sy, w, h, 0xFF606080u);

    for (int i = 0; i < item_count; i++)
    {
        int iy = sy + kPad + i * kItemH;
        g.draw_text(sx + 10, iy + 6, items[i].label ? items[i].label : "",
                    0xFFFFFFFFu, 0xFF2A2A3Au);
    }
}

bool StartMenu::on_event(const Event &e)
{
    if (!open || !visible)
        return false;

    if (e.type == EventType::MouseDown && contains_screen(e.x, e.y))
    {
        int sx, sy;
        screen_pos(sx, sy);
        int rel = e.y - sy - kPad;
        if (rel >= 0)
        {
            int idx = rel / kItemH;
            if (idx >= 0 && idx < item_count && items[idx].action)
            {
                items[idx].action(items[idx].user);
            }
        }
        close_menu();
        return true;
    }
    return false;
}
