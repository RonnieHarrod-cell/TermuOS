#pragma once
#include "events.hpp"
#include "gfx.hpp"

class Widget
{
public:
    int x = 0, y = 0, w = 0, h = 0;
    bool visible = true;
    bool dirty = true;
    Widget *parent = nullptr;

    static constexpr int kMaxChildren = 16;
    Widget *children[kMaxChildren]{};
    int child_count = 0;

    virtual ~Widget() = default;

    void add(Widget *c)
    {
        if (child_count >= kMaxChildren || !c)
            return;
        children[child_count++] = c;
        c->parent = this;
    }

    void screen_pos(int &sx, int &sy) const
    {
        sx = x;
        sy = y;
        for (Widget *p = parent; p; p = p->parent)
        {
            sx += p->x;
            sy += p->y;
        }
    }

    bool contains_screen(int sx, int sy) const
    {
        int ox, oy;
        screen_pos(ox, oy);
        return sx >= ox && sy >= oy && sx < ox + w && sy < oy + h;
    }

    virtual void paint(Gfx &g) { (void)g; }

    virtual bool accepts_focus() const { return false; }
    virtual void on_focus() { mark_dirty(); }
    virtual void on_blur() { mark_dirty(); }

    virtual bool on_event(const Event &e)
    {
        for (int i = child_count - 1; i >= 0; --i)
        {
            if (children[i] && children[i]->visible && children[i]->on_event(e))
                return true;
        }
        return false;
    }

    void paint_tree(Gfx &g)
    {
        if (!visible)
            return;
        paint(g);
        for (int i = 0; i < child_count; ++i)
            if (children[i])
                children[i]->paint_tree(g);
    }

    void mark_dirty() { dirty = true; }
};
