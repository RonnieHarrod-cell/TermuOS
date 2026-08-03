#pragma once
#include <stdint.h>

struct termuos_mouse_event
{
    int32_t x, y;
    int32_t dx, dy;
    uint8_t buttons;
};

struct termuos_key_event
{
    uint32_t keycode;
    uint8_t pressed;
    char text[4];
};
