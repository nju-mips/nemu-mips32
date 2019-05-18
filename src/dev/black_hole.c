#include "device.h"
#include "nemu.h"

uint32_t blackhole_read(paddr_t addr, int len) { return 0; }

void blackhole_write(paddr_t addr, int len, uint32_t data) {
  return;
}

device_t blackhole_dev = {
    .name = "<BAD>",
    .start = BLACKHOLE_ADDR,
    .end = BLACKHOLE_ADDR + BLACKHOLE_SIZE,
    .read = blackhole_read,
    .write = blackhole_write,
};
