#include <SDL/SDL.h>
#include <stdbool.h>

#include "device.h"
#include "utils/file.h"
#include "utils/sdlkey.h"
#include "utils/fifo.h"

#define NEMU_KEYBOARD_CODE 0x0
#define NEMU_KEYBOARD_STAT 0x4
#define NEMU_KEYBOARD_SIZE 0x10

static fifo_type(uint32_t, 1024) nemu_kbd_queue;

static bool nemu_keyboard_update_irq() {
  SDL_PumpEvents();
  SDL_Event evt = {0};
  int cnt = SDL_PeepEvents(&evt, 1, SDL_GETEVENT,
      SDL_EVENTMASK(SDL_KEYDOWN) |
          SDL_EVENTMASK(SDL_KEYUP));
  if (cnt <= 0) return false;

  uint32_t scancode =
      SDLKey_to_scancode(evt.type, evt.key.keysym.sym);
  fifo_push(nemu_kbd_queue, scancode);
  return false;
}

static uint32_t nemu_keyboard_peek(paddr_t addr, int len) {
  check_ioaddr(addr, len, NEMU_KEYBOARD_SIZE, "kb.read");
  nemu_keyboard_update_irq();
  /* CTRL not yet implemented, only allow byte read/write */
  switch (addr) {
  case NEMU_KEYBOARD_STAT:
    return !fifo_is_empty(nemu_kbd_queue);
  case NEMU_KEYBOARD_CODE:
    return fifo_top(nemu_kbd_queue);
  default:
    CPUAssert(
        false, "kb: address(0x%08x) is not readable", addr);
    break;
  }
  return 0;
}

static uint32_t nemu_keyboard_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, NEMU_KEYBOARD_SIZE, "kb.read");
  nemu_keyboard_update_irq();
  /* CTRL not yet implemented, only allow byte read/write */
  switch (addr) {
  case NEMU_KEYBOARD_STAT:
    return !fifo_is_empty(nemu_kbd_queue);
  case NEMU_KEYBOARD_CODE:
    return fifo_pop(nemu_kbd_queue);
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
    .peek = nemu_keyboard_peek,
    .update_irq = nemu_keyboard_update_irq,
};
