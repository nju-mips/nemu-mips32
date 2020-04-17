#ifndef UTILS_CONSOLE_H
#define UTILS_CONSOLE_H

#include <stdint.h>

/* console control */
void init_console();
void resume_console();
void disable_buffer();
void enable_buffer();
void echo_off();
void echo_on();
void set_cursor(uint32_t x, uint32_t y);
void hide_cursor();
void show_cursor();
int nchars_stdin();
void save_cursor_pos();
void load_cursor_pos();
void init_scr_wh(int *w, int *h);

#endif
