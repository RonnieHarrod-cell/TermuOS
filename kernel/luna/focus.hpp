#pragma once

class Widget;

class Focus
{
public:
    static Widget *current();
    static void set(Widget *w);
    static void clear();

private:
    static Widget *current_;
};
