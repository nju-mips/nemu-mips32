#include <stdlib.h>

#include "dev/device.h"

#define NEMU_CLOCK_SIZE 0x4

uint64_t get_current_time(); // us

static uint32_t nemu_clock_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, NEMU_CLOCK_SIZE, "rtc.read");
  return get_current_time() / 1000;
}

DEF_DEV(nemu_clock_dev) = {
    .name = "nemu-clock",
    .start = CONFIG_NEMU_CLOCK_BASE,
    .size = NEMU_CLOCK_SIZE,
    .read = nemu_clock_read,
    .peek = nemu_clock_read,
};
