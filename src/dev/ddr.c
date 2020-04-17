#include "device.h"

#define DDR_SIZE (128 * 1024 * 1024) // 0x08000000

static uint8_t ddr[DDR_SIZE];

/* Memory accessing interfaces */

static void *ddr_map(uint32_t addr, uint32_t len) {
  check_ioaddr(addr, len, DDR_SIZE, "ddr.map");
  return &ddr[addr];
}

static uint32_t ddr_read(paddr_t addr, int len) {
  check_ioaddr(addr, len, DDR_SIZE, "ddr.read");
  return *((uint32_t *)((void *)ddr + addr)) & (~0u >> ((4 - len) << 3));
}

static void ddr_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, len, DDR_SIZE, "ddr.write");
  memcpy((uint8_t *)ddr + addr, &data, len);
}

static void ddr_set_block_data(paddr_t addr, const void *data, int len) {
  check_ioaddr(addr, len, DDR_SIZE, "ddr.write");
  memcpy((void *)ddr + addr, data, len);
}

DEF_DEV(ddr_dev) = {
    .name = "ddr",
    .start = CONFIG_DDR_BASE,
    .size = DDR_SIZE,
    .read = ddr_read,
    .write = ddr_write,
    .map = ddr_map,
    .peek = ddr_read,
    .set_block_data = ddr_set_block_data,
};
