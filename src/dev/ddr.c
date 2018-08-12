#include "device.h"
#include "nemu.h"


uint8_t ddr[DDR_SIZE];

/* Memory accessing interfaces */

#define check_ddr(addr, len) \
  CPUAssert(addr >= 0 && addr < DDR_SIZE && addr + len <= DDR_SIZE, \
	  "address(0x%08x) is out side DDR", addr);

void *ddr_map(uint32_t addr, uint32_t size) {
#ifdef __ARCH_LOONGSON__
  if(is_uncached(addr)) {
	addr -= UNCACHED_DDR_BASE;
	Assert(addr + size <= DDR_SIZE,
		"addr + size is %08x, UNCACHED_DDR_BASE:%08x\n",
		addr + UNCACHED_DDR_BASE + size, UNCACHED_DDR_BASE);
  }
  else
#endif
  {
	addr -= DDR_BASE;
	Assert(addr <= DDR_SIZE && addr + size <= DDR_SIZE,
		"addr is %08x, DDR_BASE:%08x\n", addr + DDR_BASE, DDR_BASE);
  }
  return &ddr[addr];
}

uint32_t ddr_read(paddr_t addr, int len) {
  check_ddr(addr, len);
  return *((uint32_t *)((uint8_t *)ddr + addr)) & (~0u >> ((4 - len) << 3));
}

void ddr_write(paddr_t addr, int len, uint32_t data) {
  check_ddr(addr, len);
  memcpy((uint8_t *)ddr + addr, &data, len);
}
