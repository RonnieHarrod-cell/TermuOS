#pragma once
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int w, h;
        const uint8_t *rgba;
        size_t size;
    } luna_icon_t;

    int luna_icon_load(const char *name, int w, int h, luna_icon_t *out);

#ifdef __cplusplus
}
#endif
