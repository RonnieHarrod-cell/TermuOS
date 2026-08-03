#pragma once

#include "Graphics.hpp"

class Window
{
public:
    Window(int x, int y, int w, int h, const char *title);

    void draw(Graphics &g);
    void set_title(const char *title);
    void set_position(int nx, int ny);
    void set_size(int nw, int nh);

    int x, y, width, height;
    bool visible = true;

    static constexpr int TITLEBAR_H = 28;
    static constexpr int BORDER = 2;

private:
    char title_[64];
};
