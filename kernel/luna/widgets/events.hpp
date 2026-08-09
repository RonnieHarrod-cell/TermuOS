#pragma once
#include <stdint.h>

enum class EventType : uint8_t
{
    MouseMove,
    MouseDown,
    MouseUp,
    KeyDown,
    KeyUp
};

struct Event
{
    EventType type;
    int x, y;
    uint8_t buttons;
    char key;
    uint8_t keycode;
};
