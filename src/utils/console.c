#include <sys/ioctl.h>
#include <stdarg.h>
#include <sys/types.h>
#include <termios.h>

#include "debug.h"
#include "utils/utils.h"

#define STDIN_FILENO 0

static struct termio raw_termio;

void wrap_get_term_attr(struct termio *ptbuf) {
  if (ioctl(0, TCGETA, ptbuf) == -1) {
    eprintf("fail to get terminal information\n");
  }
}

void wrap_set_term_attr(struct termio *ptbuf) {
  if (ioctl(0, TCSETA, ptbuf) == -1) {
    eprintf("fail to set terminal information\n");
  }
}

/* functions to change terminal state */
void disable_buffer() {
  struct termio tbuf;
  wrap_get_term_attr(&tbuf);

  tbuf.c_lflag &= ~ICANON;
  tbuf.c_cc[VMIN] = raw_termio.c_cc[VMIN];

  wrap_set_term_attr(&tbuf);
}

void enable_buffer() {
  struct termio tbuf;
  wrap_get_term_attr(&tbuf);

  tbuf.c_lflag |= ICANON;
  tbuf.c_cc[VMIN] = 60;

  wrap_set_term_attr(&tbuf);
}

void echo_off() {
  struct termio tbuf;
  wrap_get_term_attr(&tbuf);

  tbuf.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);

  wrap_set_term_attr(&tbuf);
}

void echo_on() {
  struct termio tbuf;
  wrap_get_term_attr(&tbuf);

  tbuf.c_lflag |= (ECHO | ECHOE | ECHOK | ECHONL);

  wrap_set_term_attr(&tbuf);
}

void set_cursor(uint32_t x, uint32_t y) { printf("\033[%d;%df", y + 1, x + 1); }

void hide_cursor() { printf("\033[?25l"); }

void show_cursor() { printf("\033[?25h"); }

int nchars_stdin() {
  int n = 0;
  ioctl(STDIN_FILENO, FIONREAD, &n);
  return n;
}

void save_cursor_pos() { printf("\033[s"); }

void load_cursor_pos() { printf("\033u"); }

void init_scr_wh(int *w, int *h) {
  struct winsize ws;
  ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
  *w = ws.ws_col;
  *h = ws.ws_row;
}

void init_console() {
  wrap_get_term_attr(&raw_termio);
  disable_buffer();
  echo_off();
}

void resume_console() {
  echo_on();
  enable_buffer();
  wrap_set_term_attr(&raw_termio);
}
