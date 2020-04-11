#include "dev/device.h"

// block ram
#define BRAM_SIZE (1024 * 1024)

static uint8_t bram[BRAM_SIZE];

static void *bram_map(uint32_t addr, uint32_t len) {
  check_ioaddr(addr, len, BRAM_SIZE, "bram.map");
  return &bram[addr];
}

static uint32_t bram_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, BRAM_SIZE, "bram.read");
  return *((uint32_t *)((void *)bram + addr)) & (~0u >> ((4 - len) << 3));
}

static void bram_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, BRAM_SIZE, "bram.write");
  memcpy((void *)bram + addr, &data, len);
}

static void bram_set_block_data(paddr_t addr, const void *data, int len) {
  check_ioaddr(addr, len, BRAM_SIZE, "bram.write");
  memcpy((void *)bram + addr, data, len);
}

DEF_DEV(bram_dev) = {
    .name = "block-ram",
    .start = CONFIG_BRAM_BASE,
    .size = BRAM_SIZE,
    .read = bram_read,
    .write = bram_write,
    .map = bram_map,
    .peek = bram_read,
    .set_block_data = bram_set_block_data,
};
