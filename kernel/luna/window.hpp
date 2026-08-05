#include "widget.hpp"

class Window : public Widget
{
public:
    const char *title = "";
    static constexpr int kTitleH = 24;
    static constexpr int kBorder = 3;

    int client_x() const { return kBorder; }
    int client_y() const { return kBorder + kTitleH; }
    int client_w() const { return w - kBorder * 2; }
    int client_h() const { return h - kTitleH - kBorder * 2; }

    void paint(Gfx &g) override;
    bool on_event(const Event &e) override;

private:
    bool dragging_ = false;
    int drag_ox_ = 0, drag_oy_ = 0;
};
