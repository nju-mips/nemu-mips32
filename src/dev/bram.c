#include "device.h"
#include "nemu.h"

// block ram
#define BRAM_BASE 0x1fc00000
#define BRAM_SIZE (1024 * 1024)

static uint8_t bram[BRAM_SIZE];

/* fake spi flash */

void *bram_map(uint32_t addr, uint32_t len) {
  check_ioaddr(addr, BRAM_SIZE, "bram.map");
  return &bram[addr];
}

uint32_t bram_read(paddr_t addr, int len) {
  check_ioaddr(addr, BRAM_SIZE, "bram.read");
  return *((uint32_t *)((void *)bram + addr)) &
         (~0u >> ((4 - len) << 3));
}

void bram_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, BRAM_SIZE, "bram.write");
  memcpy((void *)bram + addr, &data, len);
}

device_t bram_dev = {
    .name = "BRAM",
    .start = BRAM_BASE,
    .end = BRAM_BASE + BRAM_SIZE,
    .read = bram_read,
    .write = bram_write,
    .map = bram_map,
    .peek = bram_read,
};
