#pragma once
#include "../widgets/widget.hpp"
#include "wm.hpp"
#include "../focus.hpp"

class Taskbar;

class Desktop : public Widget
{
public:
    uint32_t wallpaper = 0xFF1A1A2Eu;
    Wm *wm = nullptr;
    Taskbar *taskbar = nullptr;

    void paint(Gfx &g) override
    {
        g.fill_rect(0, 0, w, h, wallpaper);
        /* icon children via paint_tree */
    }

    bool on_event(const Event &e) override
    {
        if (Widget::on_event(e))
            return true;
        if (e.type == EventType::MouseDown)
        {
            Focus::clear();
            mark_dirty();
            return true;
        }
        return false;
    }
};
