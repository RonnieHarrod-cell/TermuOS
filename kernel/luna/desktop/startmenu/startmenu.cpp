#include "startmenu.hpp"
#include "../../focus.hpp"

static const char *cat_or_default(const char *c)
{
    return (c && c[0]) ? c : "General";
}

bool StartMenu::same_category(const char *a, const char *b)
{
    const char *ca = cat_or_default(a);
    const char *cb = cat_or_default(b);
    while (*ca && *ca == *cb)
    {
        ca++;
        cb++;
    }
    return *ca == *cb;
}

void StartMenu::add_item(const char *label, const char *category,
                         void (*action)(void *), void *user)
{
    if (item_count >= kMaxItems)
        return;
    items[item_count].label = label;
    items[item_count].category = category;
    items[item_count].action = action;
    items[item_count].user = user;
    item_count++;
}

int StartMenu::item_y(int index) const
{
    int y = 0;
    for (int i = 0; i < index; i++)
    {
        y += kItemH;
        if (i + 1 < item_count &&
            !same_category(items[i].category, items[i + 1].category))
            y += kSepH;
    }
    return y;
}

int StartMenu::content_height() const
{
    if (item_count == 0)
        return 0;
    return item_y(item_count - 1) + kItemH;
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
        w = 180;
        h = kPad * 2 + content_height();
        x = 4;
        y = screen_h - 36 - h - 4;
        if (y < 4)
            y = 4;
    }
    mark_dirty();
}

void StartMenu::paint(Gfx &g)
{
    if (!open || !visible)
        return;

    int sx, sy;
    screen_pos(sx, sy);

    g.fill_rect(sx, sy, w, h, 0xFF161C2Au);
    g.draw_rect(sx, sy, w, h, 0xFF5B8CFFu);

    for (int i = 0; i < item_count; i++)
    {
        int iy = sy + kPad + item_y(i);

        if (i > 0 &&
            !same_category(items[i - 1].category, items[i].category))
        {
            int sep_y = iy - kSepH / 2;
            g.fill_rect(sx + 10, sep_y, w - 20, 1, 0xFF3A455Fu);
        }

        g.draw_text(sx + 12, iy + 6,
                    items[i].label ? items[i].label : "",
                    0xFFE8ECF4u, 0xFF161C2Au);
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

        for (int i = 0; i < item_count; i++)
        {
            int iy = item_y(i);
            if (rel >= iy && rel < iy + kItemH)
            {
                if (items[i].action)
                    items[i].action(items[i].user);
                break;
            }
        }
        close_menu();
        return true;
    }
    return false;
}
