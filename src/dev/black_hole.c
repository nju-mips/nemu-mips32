#include "device.h"
#include "nemu.h"

uint32_t badp_read(paddr_t addr, int len) { return 0; }

void badp_write(paddr_t addr, int len, uint32_t data) {
  return;
}

device_t blackhole_dev = {
    .name = "<bad>",
    .start = BADP_ADDR,
    .end = BADP_ADDR + BADP_SIZE,
    .read = badp_read,
    .write = badp_write,
};
