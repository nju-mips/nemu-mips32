#include "device.h"
#include "nemu.h"


uint8_t bram[BRAM_SIZE];

/* Memory accessing interfaces */

#define check_bram(addr, len) \
  CPUAssert(addr >= 0 && addr < BRAM_SIZE && addr + len <= BRAM_SIZE, \
	  "address(0x%08x) is out side BRAM", addr);

void *bram_map(uint32_t addr, uint32_t len) {
  check_bram(addr, len);
  return &bram[addr];
}

uint32_t bram_read(paddr_t addr, int len) {
  check_bram(addr, len);
  return *((uint32_t *)((void*)bram + addr)) & (~0u >> ((4 - len) << 3));
}

void bram_write(paddr_t addr, int len, uint32_t data) {
  check_bram(addr, len);
  memcpy((uint8_t *)bram + addr, &data, len);
}
