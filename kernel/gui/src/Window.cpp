#include "../include/Window.hpp"

static void copy_str(char *dst, const char *src, int max)
{
    int i = 0;
    if (src)
    {
        while (src[i] && i < max - 1)
        {
            dst[i] = src[i];
            i++;
        }
    }
    dst[i] = '\0';
}

Window::Window(int x, int y, int w, int h, const char *title)
    : x(x), y(y), width(w), height(h)
{
    copy_str(title_, title, sizeof(title_));
}

void Window::set_title(const char *title)
{
    copy_str(title_, title, sizeof(title_));
}

void Window::set_position(int nx, int ny)
{
    x = nx;
    y = ny;
}

void Window::set_size(int nw, int nh)
{
    width = nw;
    height = nh;
}

void Window::draw(Graphics &g)
{
    if (!visible || width <= 0 || height <= 0)
        return;

    // Colours
    uint32_t title_bg = g.colour(0x2D, 0x2D, 0x3A);
    uint32_t title_fg = g.colour(0xE0, 0xE0, 0xE0);
    uint32_t border_col = g.colour(0x4A, 0x4A, 0x5A);
    uint32_t body_bg = g.colour(0x1A, 0x1A, 0x24);
    uint32_t close_col = g.colour(0xE0, 0x55, 0x55);

    // Outer border
    g.draw_rect(x, y, width, height, border_col);

    // Title bar
    g.fill_rect(x + BORDER, y + BORDER,
                width - BORDER * 2, TITLEBAR_H - BORDER, title_bg);

    // Body
    int body_y = y + TITLEBAR_H;
    int body_h = height - TITLEBAR_H - BORDER;
    if (body_h > 0)
        g.fill_rect(x + BORDER, body_y, width - BORDER * 2, body_h, body_bg);

    // Title text
    int text_x = x + 10;
    int text_y = y + (TITLEBAR_H - Graphics::FONT_H) / 2;
    g.draw_string(text_x, text_y, title_, title_fg, title_bg);

    // Close button
    int btn_size = 12;
    int btn_x = x + width - BORDER - btn_size - 8;
    int btn_y = y + (TITLEBAR_H - btn_size) / 2;
    g.fill_rect(btn_x, btn_y, btn_size, btn_size, close_col);
}
