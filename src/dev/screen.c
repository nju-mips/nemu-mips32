#if CONFIG_VGA_CONTROLLER
#  include <SDL/SDL.h>

#  include "device.h"

extern SDL_Surface *screen;

static uint32_t screen_read(paddr_t addr, int len) {
  assert(addr == 0);
  /* get the width and height */
  return (SCR_W << 16) + SCR_H;
}

static void screen_write(paddr_t addr, int len, uint32_t data) {
  /* sync the screen */
  assert(addr == 4);
  SDL_Flip(screen);
}

DEF_DEV(screen_dev) = {
    .name = "SCREEN_CONFIG",
    .start = CONFIG_VGA_CONTROLLER_BASE,
    .end = CONFIG_VGA_CONTROLLER_BASE + 4,
    .peek = screen_read,
    .read = screen_read,
    .write = screen_write,
};
#endif
