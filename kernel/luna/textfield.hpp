#pragma once
#include "widget.hpp"

class TextField : public Widget
{
public:
    static constexpr int kCap = 128;

    char buffer[kCap]{};
    int length = 0;
    int caret = 0;

    uint32_t fg = 0xFF000000u;
    uint32_t bg = 0xFFFFFFFFu;
    uint32_t border = 0xFF404040u;
    uint32_t border_focus = 0xFF2D5B8Au;

    bool accepts_focus() const override { return true; }
    void paint(Gfx &g) override;
    bool on_event(const Event &e) override;
    void on_focus() override;
    void on_blur() override;

private:
    bool focused_ = false;
};
