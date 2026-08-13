#pragma once
#include <stdint.h>

namespace Theme
{
    constexpr uint32_t desktop = 0xFF008080u;   // teal
    constexpr uint32_t face = 0xFFC0C0C0u;      // 3D face
    constexpr uint32_t highlight = 0xFFFFFFFFu; // top-left
    constexpr uint32_t shadow = 0xFF808080u;    // bottom-right
    constexpr uint32_t dkshadow = 0xFF000000u;
    constexpr uint32_t title_left = 0xFF000080u;  // navy
    constexpr uint32_t title_right = 0xFF1084D0u; // blue
    constexpr uint32_t title_text = 0xFFFFFFFFu;
    constexpr uint32_t title_inact = 0xFF808080u;
    constexpr uint32_t client = 0xFFFFFFFFu; // window body
    constexpr uint32_t select = 0xFF000080u;
    constexpr uint32_t select_text = 0xFFFFFFFFu;
    constexpr uint32_t text = 0xFF000000u;
}
