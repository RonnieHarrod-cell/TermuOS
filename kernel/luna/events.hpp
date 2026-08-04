#pragma once
#include <stdint.h>

enum class EventType : uint8_t
{
    MouseMove,
    MouseDown,
    MouseUp,
    Key
};

struct Event
{
    EventType type;
    int x, y;
    uint8_t buttons;
    char key;
};
