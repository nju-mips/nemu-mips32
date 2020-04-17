#include <SDL/SDL.h>
#include <stdbool.h>

#include "dev/device.h"
#include "dev/events.h"
#include "utils/file.h"

#define NEMU_KEYBOARD_CODE 0x0
#define NEMU_KEYBOARD_STAT 0x4
#define NEMU_KEYBOARD_SIZE 0x10

/////////////////////////////////////////////////////////////////
//                     kb simulation //
/////////////////////////////////////////////////////////////////

#define KEYBOARD_QUEUE_LEN 1024
static int nemu_keyboard_queue[KEYBOARD_QUEUE_LEN];
static int nemu_keyboard_f = 0, nemu_keyboard_r = 0;

static bool nemu_keyboard_update_irq() {
  SDL_Event evt = {0};
  int cnt = SDL_PeepEvents(&evt, 1, SDL_GETEVENT,
      SDL_EVENTMASK(SDL_KEYDOWN) |
          SDL_EVENTMASK(SDL_KEYUP));
  if (cnt <= 0) return false;

  uint32_t scancode =
      SDLKey_to_scancode(evt.type, evt.key.keysym.sym);
  int next = (nemu_keyboard_r + 1) % KEYBOARD_QUEUE_LEN;
  if (next != nemu_keyboard_f) { // if not full
    nemu_keyboard_queue[nemu_keyboard_r] = scancode;
    nemu_keyboard_r = next;
    /* no irq */
  }
  return false;
}

static uint32_t nemu_keyboard_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, NEMU_KEYBOARD_SIZE, "kb.read");
  /* CTRL not yet implemented, only allow byte read/write */
  switch (addr) {
  case NEMU_KEYBOARD_STAT:
    return nemu_keyboard_f == nemu_keyboard_r ? 0 : 1;
  case NEMU_KEYBOARD_CODE: {
    uint32_t code = nemu_keyboard_queue[nemu_keyboard_f];
    nemu_keyboard_f =
        (nemu_keyboard_f + 1) % KEYBOARD_QUEUE_LEN;
    return code;
  }
  default:
    CPUAssert(
        false, "kb: address(0x%08x) is not readable", addr);
    break;
  }
  return 0;
}

DEF_DEV(nemu_keyboard_dev) = {
    .name = "nemu-keyboard",
    .start = CONFIG_NEMU_KEYBOARD_BASE,
    .size = NEMU_KEYBOARD_SIZE,
    .read = nemu_keyboard_read,
    .update_irq = nemu_keyboard_update_irq,
};
