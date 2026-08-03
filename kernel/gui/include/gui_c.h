#pragma once

#include <stdint.h>
#include <limine.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void gui_init(struct limine_framebuffer *fb);
    void gui_update(void);
    void gui_add_test_window(void);

#ifdef __cplusplus
}
#endif
