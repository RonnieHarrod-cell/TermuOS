#include "focus.hpp"
#include "widgets/widget.hpp"

Widget *Focus::current_ = nullptr;

Widget *Focus::current()
{
    return current_;
}

void Focus::set(Widget *w)
{
    if (current_ == w)
        return;
    if (current_)
        current_->on_blur();
    current_ = w;
    if (current_)
        current_->on_focus();
}

void Focus::clear()
{
    set(nullptr);
}
