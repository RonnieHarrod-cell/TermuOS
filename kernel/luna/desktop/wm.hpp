#pragma once
#include "../widgets/window.hpp"

class Wm
{
public:
    static constexpr int kMax = 8;
    Window *stack[kMax]{};
    int count = 0;

    bool add(Window *w)
    {
        if (!w || count >= kMax)
            return false;
        stack[count++] = w;
        return true;
    }

    void remove(Window *w)
    {
        for (int i = 0; i < count; i++)
        {
            if (stack[i] != w)
                continue;
            for (int j = i; j < count - 1; j++)
                stack[j] = stack[j + 1];
            count--;
            w->visible = false;
            return;
        }
    }

    void raise(Window *w)
    {
        int i = 0;
        for (; i < count; i++)
            if (stack[i] == w)
                break;
        if (i >= count)
            return;
        for (int j = i; j < count - 1; j++)
            stack[j] = stack[j + 1];
        stack[count - 1] = w;
    }

    Window *hit(int sx, int sy)
    {
        for (int i = count - 1; i >= 0; i--)
        {
            Window *w = stack[i];
            if (w && w->visible && w->contains_screen(sx, sy))
                return w;
        }
        return nullptr;
    }

    void paint_all(Gfx &g)
    {
        for (int i = 0; i < count; i++)
            if (stack[i] && stack[i]->visible)
                stack[i]->paint_tree(g);
    }
};
