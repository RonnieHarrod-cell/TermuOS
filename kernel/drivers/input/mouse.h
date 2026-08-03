#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int32_t x, y;
        uint8_t buttons;
        int8_t dx, dy;
    } mouse_state_t;

    void mouse_init(void);
    void mouse_get_state(mouse_state_t *out);

    void mouse_set_bounds(int width, int height);

    int mouse_is_dirty(void);

#ifdef __cplusplus
}
#endif
