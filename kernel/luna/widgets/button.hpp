#include "widget.hpp"

class Button : public Widget
{
public:
    const char *text = "OK";
    bool pressed = false;
    void (*on_click)(void *) = nullptr;
    void *on_click_user = nullptr;

    void paint(Gfx &g) override;
    bool on_event(const Event &e) override;
};
