#pragma once
#include <stdint.h>

void terminal_init(void);
void terminal_set_size(uint64_t w, uint64_t h);
void terminal_set_fg(uint8_t r, uint8_t g, uint8_t b);
void terminal_set_bg(uint8_t r, uint8_t g, uint8_t b);
void terminal_putchar(char c);
void terminal_puts(const char *s);
void terminal_cursor_tick(void);

void terminal_set_offset(uint64_t x, uint64_t y);
void terminal_set_size_from_current(void);

void terminal_redraw(void);
void terminal_bind(int x, int y, int w, int h);