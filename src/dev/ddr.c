#include "device.h"
#include "nemu.h"

#define DDR_BASE (0x00000000)
#define DDR_SIZE (128 * 1024 * 1024) // 0x08000000

uint8_t ddr[DDR_SIZE];
static uint32_t ddr_mapped_size = 0;

/* Memory accessing interfaces */

void *ddr_map(uint32_t addr, uint32_t len) {
  check_ioaddr(addr, DDR_SIZE, "ddr");
  if (addr + len >= ddr_mapped_size)
    ddr_mapped_size = addr + len;
  return &ddr[addr];
}

void ddr_mapped_result(map_result_t *map) {
  map->p = ddr;
  map->size = ddr_mapped_size;
}

uint32_t ddr_read(paddr_t addr, int len) {
  check_ioaddr(addr, DDR_SIZE, "ddr");
  return *((uint32_t *)((void *)ddr + addr)) &
         (~0u >> ((4 - len) << 3));
}

void ddr_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, DDR_SIZE, "ddr");
  memcpy((uint8_t *)ddr + addr, &data, len);
}

device_t ddr_dev = {
    .name = "DDR",
    .start = DDR_BASE,
    .end = DDR_BASE + DDR_SIZE,
    .read = ddr_read,
    .write = ddr_write,
    .map = ddr_map,
    .peek = ddr_read,
};
