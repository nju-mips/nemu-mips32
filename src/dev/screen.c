#include "device.h"
#include "nemu.h"
#include <SDL/SDL.h>

#define SCREEN_ADDR 0x10003000

extern SDL_Surface *screen;

uint32_t screen_read(paddr_t addr, int len) {
  assert (addr == 0);
  /* get the width and height */
  return (SCR_W << 16) + SCR_H;
}

void screen_write(paddr_t addr, int len,
                       uint32_t data) {
  /* sync the screen */
  assert (addr == 4);
  SDL_Flip(screen);
}

device_t screen_dev = {
    .name = "SCREEN_CONFIG",
    .start = SCREEN_ADDR,
    .end = SCREEN_ADDR + 4,
    .read = screen_read,
    .write = screen_write,
};
