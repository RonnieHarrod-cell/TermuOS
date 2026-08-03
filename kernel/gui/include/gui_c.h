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

    void gui_set_mouse_pos(int x, int y);
    void gui_handle_mouse(void); /* poll mouse + redraw cursor */

    void gui_thread_entry(void);

#ifdef __cplusplus
}
#endif
