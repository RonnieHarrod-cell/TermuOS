#pragma once
#include <stdint.h>

struct termuos_fb_info
{
    uint64_t width, height, pitch;
    uint32_t bpp;
};
