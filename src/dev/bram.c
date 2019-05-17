#include "device.h"
#include "nemu.h"

uint8_t bram[BRAM_SIZE];
static uint32_t bram_mapped_size = 0;
bool bram_mapped = false;

/* fake spi flash */

void *bram_map(uint32_t addr, uint32_t len) {
  check_ioaddr(addr, BRAM_SIZE, "bram");
  if (addr == 0) bram_mapped = true;
  if (addr + len >= bram_mapped_size)
    bram_mapped_size = addr + len;
  return &bram[addr];
}

void bram_mapped_result(map_result_t *map) {
  map->p = bram;
  map->size = bram_mapped_size;
}

uint32_t bram_read(paddr_t addr, int len) {
  check_ioaddr(addr, BRAM_SIZE, "bram");
  return *((uint32_t *)((void *)bram + addr)) &
         (~0u >> ((4 - len) << 3));
}

void bram_write(paddr_t addr, int len, uint32_t data) {
  check_ioaddr(addr, BRAM_SIZE, "bram");
  memcpy((void *)bram + addr, &data, len);
}

void bram_init(vaddr_t entry) {
  if (!bram_mapped) {
    uint32_t *p = (void *)bram;
    p[0] = 0x3c080000 | (entry >> 16); // lui t0, %hi(entry)
    p[1] = 0x25080000 |
           (entry & 0xFFFF); // addiu t0, t0, %lo(entry)
    p[2] = 0x01000008;       // jr t0
    p[3] = 0x00000000;       // nop
  }
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
