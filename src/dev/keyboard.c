#if CONFIG_KEYBOARD
#  include <SDL/SDL.h>
#  include <stdbool.h>

#  include "device.h"
#  include "events.h"

#  define KB_ADDR 0x1fe94000
#  define KB_CODE 0x0
#  define KB_STAT 0x4
#  define KB_SIZE 0x10

/////////////////////////////////////////////////////////////////
//                     kb simulation //
/////////////////////////////////////////////////////////////////

#  define KEYBOARD_QUEUE_LEN 1024
static int kb_queue[KEYBOARD_QUEUE_LEN];
static int kb_f = 0, kb_r = 0;

static void kb_on_data(void *data, int len) {
  assert(len == 2 * sizeof(data));
  int *sdlk_data = data;
  uint32_t scancode = SDLKey_to_scancode(sdlk_data[0], sdlk_data[1]);
  int next = (kb_r + 1) % KEYBOARD_QUEUE_LEN;
  if (next != kb_f) { // if not full
    kb_queue[kb_r] = scancode;
    kb_r = next;
    /* no irq */
  }
}

static uint32_t kb_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, KB_SIZE, "kb.read");
  /* CTRL not yet implemented, only allow byte read/write */
  switch (addr) {
  case KB_STAT: return kb_f == kb_r ? 0 : 1;
  case KB_CODE: {
    uint32_t code = kb_queue[kb_f];
    kb_f = (kb_f + 1) % KEYBOARD_QUEUE_LEN;
    return code;
  }
  default: CPUAssert(false, "kb: address(0x%08x) is not readable", addr); break;
  }
  return 0;
}

static void kb_init();

DEF_DEV(kb_dev) = {
    .name = "KEYBOARD",
    .init = kb_init,
    .start = KB_ADDR,
    .end = KB_ADDR + KB_SIZE,
    .read = kb_read,
    .on_data = kb_on_data,
};

static void kb_init() {
  device_bind_event(kv_dev, EVENT_SDL_KEYDOWN);
  device_bind_event(kv_dev, EVENT_SDL_KEYUP);
}
#endif
