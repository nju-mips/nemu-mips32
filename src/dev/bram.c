#include "nemu.h"
#include "device.h"
#include "cpu/mmu.h"


uint8_t bram[BRAM_SIZE];
static uint32_t bram_mapped_size = 0;
bool bram_mapped = false;

/* fake spi flash */

#define check_bram(addr, len) \
  CoreAssert(addr >= 0 && addr < BRAM_SIZE && addr + len <= BRAM_SIZE, \
	  "address(0x%08x) is out side BRAM", addr);


void *bram_map(uint32_t addr, uint32_t len) {
  check_bram(addr, len);
  if(addr == 0) bram_mapped = true;
  if(addr + len >= bram_mapped_size)
	bram_mapped_size = addr + len;
  return &bram[addr];
}

void bram_mapped_result(mmio_image_t *map) {
  map->p = bram;
  map->size = bram_mapped_size;
}

uint32_t bram_read(paddr_t addr, int len) {
  check_bram(addr, len);
  return read_masked_word(bram, addr, len);
}

void bram_write(paddr_t addr, int len, uint32_t data) {
  check_bram(addr, len);
  write_masked_word(bram, addr, len, data);
}

void bram_init(vaddr_t entry) {
  if(!bram_mapped) {
	uint32_t *p = (uint32_t*)bram;
	p[0] = 0x3c080000 | (entry >> 16);    // lui t0, %hi(entry)
	p[1] = 0x25080000 | (entry & 0xFFFF); // addiu t0, t0, %lo(entry)
	p[2] = 0x01000008;  // jr t0
	p[3] = 0x00000000;  // nop
  }
}
