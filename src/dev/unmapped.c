#include "common.h"
#include "memory.h"
#include "device.h"

uint8_t unmapped[UNMAPPED_SIZE];

#define check_unmapped(addr, len) \
  CPUAssert(addr >= 0 && addr < UNMAPPED_SIZE && addr + len <= UNMAPPED_SIZE, \
	  "address(0x%08x) is out side UNMAPPED", addr);

void *unmapped_map(uint32_t addr, uint32_t size) {
  addr -= UNMAPPED_BASE;
  Assert(addr <= UNMAPPED_SIZE && addr + size <= UNMAPPED_SIZE,
	  "addr is %08x, UNMAPPED_BASE:%08x\n", addr + UNMAPPED_BASE, UNMAPPED_BASE);
  return &unmapped[addr];
}

uint32_t unmapped_read(paddr_t addr, int len) {
  check_unmapped(addr, len);
  eprintf("unmapped read %08x -- %08x\n", addr, addr + UNMAPPED_BASE);
  return *((uint32_t *)((uint8_t *)unmapped + addr)) & (~0u >> ((4 - len) << 3));
}

void unmapped_write(paddr_t addr, int len, uint32_t data) {
  check_unmapped(addr, len);
  eprintf("unmapped write %08x -- %08x\n", addr, addr + UNMAPPED_BASE);
  memcpy((uint8_t *)unmapped + addr, &data, len);
}
