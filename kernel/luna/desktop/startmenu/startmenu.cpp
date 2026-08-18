#include "startmenu.hpp"
#include "../../theme.hpp"
#include "../../focus.hpp"

static const char *cat_name(const char *c)
{
    return (c && c[0]) ? c : "General";
}

static bool str_eq(const char *a, const char *b)
{
    if (!a || !b)
        return a == b;
    while (*a && *a == *b)
    {
        a++;
        b++;
    }
    return *a == *b;
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

void StartMenu::rebuild_categories()
{
    cat_count = 0;
    for (int i = 0; i < item_count; i++)
    {
        const char *c = cat_name(items[i].category);
        int found = -1;
        for (int j = 0; j < cat_count; j++)
        {
            if (str_eq(cats[j].name, c))
            {
                found = j;
                break;
            }
        }
        if (found < 0)
        {
            if (cat_count >= kMaxCats)
                continue;
            cats[cat_count].name = c;
            cats[cat_count].count = 1;
            cat_count++;
        }
        else
        {
            cats[found].count++;
        }
    }
}

int StartMenu::root_height() const
{
    return kPad * 2 + cat_count * kItemH;
}

int StartMenu::sub_height(int cat) const
{
    if (cat < 0 || cat >= cat_count)
        return 0;
    return kPad * 2 + cats[cat].count * kItemH;
}

void StartMenu::relayout()
{
    int taskbar = 36;
    int rh = root_height();

    w = kPanelW;
    h = rh;
    x = 2;
    y = screen_h_ - taskbar - rh - 2;
    if (y < 2)
        y = 2;

    sub_offset_y = 0;

    if (open_cat < 0 || open_cat >= cat_count)
        return;

    int sub_h = sub_height(open_cat);
    sub_offset_y = kPad + open_cat * kItemH;

    int abs_sub_y = y + sub_offset_y;
    int bottom = screen_h_ - taskbar - 2;
    if (abs_sub_y + sub_h > bottom)
        abs_sub_y = bottom - sub_h;
    if (abs_sub_y < 2)
        abs_sub_y = 2;

    sub_offset_y = abs_sub_y - y;

    w = kPanelW * 2 - 2;

    int need_h = sub_offset_y + sub_h;
    if (need_h > h)
        h = need_h;

    if (sub_offset_y < 0)
    {
        y += sub_offset_y;
        h -= sub_offset_y;
        sub_offset_y = 0;
        if (y < 2)
        {
            h -= (2 - y);
            y = 2;
        }
    }
}

void StartMenu::close_menu()
{
    open = false;
    visible = false;
    open_cat = -1;
    w = kPanelW;
    mark_dirty();
}

void StartMenu::toggle(int screen_h)
{
    screen_h_ = screen_h;
    open = !open;
    visible = open;
    open_cat = -1;
    if (open)
    {
        rebuild_categories();
        relayout();
    }
    else
    {
        w = kPanelW;
    }
    mark_dirty();
}

void StartMenu::paint(Gfx &g)
{
    if (!open || !visible)
        return;

    int sx, sy;
    screen_pos(sx, sy);

    int rh = root_height();
    g.draw_raised(sx, sy, kPanelW, rh);

    for (int i = 0; i < cat_count; i++)
    {
        int iy = sy + kPad + i * kItemH;
        bool sel = (i == open_cat);
        uint32_t bg = sel ? Theme::select : Theme::face;
        uint32_t fg = sel ? Theme::select_text : Theme::text;
        if (sel)
            g.fill_rect(sx + 2, iy, kPanelW - 4, kItemH, bg);
        g.draw_text(sx + 10, iy + 6, cats[i].name ? cats[i].name : "", fg, bg);
        g.draw_text(sx + kPanelW - 16, iy + 6, ">", fg, bg);
    }

    if (open_cat < 0 || open_cat >= cat_count)
        return;

    int sub_h = sub_height(open_cat);
    int sub_x = sx + kPanelW - 2;
    int sub_y = sy + sub_offset_y;

    g.draw_raised(sub_x, sub_y, kPanelW, sub_h);

    int row = 0;
    const char *want = cats[open_cat].name;
    for (int i = 0; i < item_count; i++)
    {
        if (!str_eq(cat_name(items[i].category), want))
            continue;
        int iy = sub_y + kPad + row * kItemH;
        g.draw_text(sub_x + 10, iy + 6,
                    items[i].label ? items[i].label : "",
                    Theme::text, Theme::face);
        row++;
    }
}

bool StartMenu::on_event(const Event &e)
{
    if (!open || !visible)
        return false;

    int sx, sy;
    screen_pos(sx, sy);

    int rh = root_height();
    int sub_h = (open_cat >= 0) ? sub_height(open_cat) : 0;
    int sub_x = sx + kPanelW - 2;
    int sub_y = sy + sub_offset_y;

    auto in_root = [&](int px, int py)
    {
        return px >= sx && px < sx + kPanelW && py >= sy && py < sy + rh;
    };
    auto in_sub = [&](int px, int py)
    {
        if (open_cat < 0)
            return false;
        return px >= sub_x && px < sub_x + kPanelW &&
               py >= sub_y && py < sub_y + sub_h;
    };

    if (e.type == EventType::MouseMove)
    {
        if (in_root(e.x, e.y) || in_sub(e.x, e.y))
            return true;
        return false;
    }

    if (e.type != EventType::MouseDown)
        return false;

    /* submenu app click */
    if (in_sub(e.x, e.y))
    {
        int rel = e.y - sub_y - kPad;
        if (rel >= 0)
        {
            int row = rel / kItemH;
            int row_i = 0;
            const char *want = cats[open_cat].name;
            for (int i = 0; i < item_count; i++)
            {
                if (!str_eq(cat_name(items[i].category), want))
                    continue;
                if (row_i == row)
                {
                    if (items[i].action)
                        items[i].action(items[i].user);
                    close_menu();
                    return true;
                }
                row_i++;
            }
        }
        return true;
    }

    /* root category click */
    if (in_root(e.x, e.y))
    {
        int rel = e.y - sy - kPad;
        if (rel >= 0)
        {
            int idx = rel / kItemH;
            if (idx >= 0 && idx < cat_count)
            {
                if (open_cat == idx)
                {
                    open_cat = -1;
                    w = kPanelW;
                }
                else
                {
                    open_cat = idx;
                    w = kPanelW * 2; /* include flyout for contains_screen */
                }
                mark_dirty();
            }
        }
        return true;
    }

    return false;
}
