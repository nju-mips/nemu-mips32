#include "device.h"

// bad phsical address
#define BLACKHOLE_ADDR 0x1fe96000
#define BLACKHOLE_SIZE 0x1000

static uint32_t blackhole_read(paddr_t addr, int len) { return 0; }

static void blackhole_write(paddr_t addr, int len, uint32_t data) { return; }

DEF_DEV(blackhole_dev) = {
    .name = "<nemu-tlb-hole>",
    .start = BLACKHOLE_ADDR,
    .end = BLACKHOLE_ADDR + BLACKHOLE_SIZE,
    .read = blackhole_read,
    .write = blackhole_write,
};
