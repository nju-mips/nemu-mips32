#include "nemu.h"
#include "device.h"
#include "cpu/mmu.h"


uint8_t ddr[DDR_SIZE];
static uint32_t ddr_mapped_size = 0;

/* Memory accessing interfaces */

#define check_ddr(addr, len) \
  CoreAssert(addr >= 0 && addr < DDR_SIZE && addr + len <= DDR_SIZE, \
	  "address(0x%08x) is out side DDR", addr);

void *ddr_map(uint32_t addr, uint32_t len) {
  check_ddr(addr, len);
  if(addr + len >= ddr_mapped_size)
	ddr_mapped_size = addr + len;
  return &ddr[addr];
}

void ddr_mapped_result(map_result_t *map) {
  map->p = ddr;
  map->size = ddr_mapped_size;
}

uint32_t ddr_read(paddr_t addr, int len) {
  check_ddr(addr, len);
  return read_masked_word(ddr, addr, len);
}

void ddr_write(paddr_t addr, int len, uint32_t data) {
  check_ddr(addr, len);
  write_masked_word(ddr, addr, len, data);
}
