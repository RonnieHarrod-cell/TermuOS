#pragma once
#include "../../widgets/widget.hpp"

class Wm;

struct StartItem
{
    const char *label;
    void (*action)(void *user);
    void *user;
};

class StartMenu : public Widget
{
public:
    static constexpr int kMaxItems = 8;
    static constexpr int kItemH = 28;
    static constexpr int kPad = 4;

    StartItem items[kMaxItems]{};
    int item_count = 0;
    bool open = false;

    void add_item(const char *label, void (*action)(void *), void *user = nullptr);
    void toggle(int screen_h);
    void close_menu();

    void paint(Gfx &g) override;
    bool on_event(const Event &e) override;
};
