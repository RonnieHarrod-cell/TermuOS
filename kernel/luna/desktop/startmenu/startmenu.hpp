#pragma once
#include "../../widgets/widget.hpp"

struct StartItem
{
    const char *label;
    const char *category;
    void (*action)(void *user);
    void *user;
};

class StartMenu : public Widget
{
public:
    static constexpr int kMaxItems = 16;
    static constexpr int kMaxCats = 8;
    static constexpr int kItemH = 28;
    static constexpr int kPad = 4;
    static constexpr int kPanelW = 140;

    StartItem items[kMaxItems]{};
    int item_count = 0;
    bool open = false;

    /* -1 = only root categories; >=0 = that category index is expanded */
    int open_cat = -1;

    void add_item(const char *label, const char *category,
                  void (*action)(void *), void *user = nullptr);
    void toggle(int screen_h);
    void close_menu();

    void paint(Gfx &g) override;
    bool on_event(const Event &e) override;

private:
    struct Cat
    {
        const char *name;
        int first_item; /* index into items[] */
        int count;
    };
    Cat cats[kMaxCats]{};
    int cat_count = 0;

    void rebuild_categories();
    int root_height() const;
    int sub_height(int cat) const;

    int screen_h_ = 600;
    int sub_offset_y = 0;
    void relayout();
};
