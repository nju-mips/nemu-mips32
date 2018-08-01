#include <stdlib.h>
#include <SDL/SDL.h>
#include "nemu.h"
#include "monitor.h"
#include "device.h"
#include "memory.h"


// the memory mapping of mips32-npc
//0x00000000 - 0x00001fff: bram
//0x10000000 - 0x1fffffff: ddr
//0x40000000 - 0x40000fff: gpio-trap
//0x40001000 - 0x40001fff: uartlite
//0x40010000 - 0x4001ffff: vga
struct mmap_region {
  uint32_t start, end;
  read_func read;
  write_func write;
} mmap_table [] = {
  // {0x00000000, 0x00001fff, invalid_read, invalid_write},
  {DDR_BASE, DDR_BASE + DDR_SIZE, ddr_read, ddr_write},
  {GPIO_BASE, GPIO_BASE + GPIO_SIZE, invalid_read, gpio_write},
  {UARTLITE_ADDR, UARTLITE_ADDR + UARTLITE_SIZE, uartlite_read, uartlite_write},
  {KB_ADDR, KB_ADDR + KB_SIZE, kb_read, invalid_write},
  {VGA_BASE, VGA_BASE + VGA_SIZE, vga_read, vga_write},
  {UNMAPPED_BASE, UNMAPPED_BASE + UNMAPPED_SIZE, unmapped_read, unmapped_write},
};

#define NR_REGION (sizeof(mmap_table) / sizeof(mmap_table[0]))

uint32_t find_region(vaddr_t addr) {
  int ret = -1;
  for(int i = 0; i < NR_REGION; i++)
	if(addr >= mmap_table[i].start && addr < mmap_table[i].end) {
	  ret = i;
	  break;
	}
  return ret;
}

uint32_t vaddr_read_safe(vaddr_t addr, int len) {
  addr = prot_addr(addr);
  int idx = find_region(addr);
  if(idx == -1) return 0;
  return mmap_table[idx].read(addr - mmap_table[idx].start, len);
}

void vaddr_write_safe(vaddr_t addr, int len, uint32_t data) {
  addr = prot_addr(addr);
  int idx = find_region(addr);
  if(idx == -1) return;
  return mmap_table[idx].write(addr - mmap_table[idx].start, len, data);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  addr = prot_addr(addr);
  int idx = find_region(addr);
  CPUAssert(idx != -1, "address(0x%08x:0x%08x) is out of bound, pc(0x%08x)\n", addr, addr, cpu.pc);
  return mmap_table[idx].read(addr - mmap_table[idx].start, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  addr = prot_addr(addr);
  int idx = find_region(addr);
  CPUAssert(idx != -1, "address(0x%08x:0x%08x) is out of bound, pc(0x%08x)\n", addr, addr, cpu.pc);
  return mmap_table[idx].write(addr - mmap_table[idx].start, len, data);
}


uint32_t invalid_read(paddr_t addr, int len) {
  CPUAssert(false, "invalid read at address(0x%08x), pc(0x%08x)\n", addr, cpu.pc);
}

void invalid_write(paddr_t addr, int len, uint32_t data) {
  CPUAssert(false, "invalid write at address(0x%08x), pc(0x%08x)\n", addr, cpu.pc);
}

static inline bool region_collide(uint32_t x, uint32_t y,
	uint32_t a, uint32_t b) {
  return (x <= a && a < y)
	|| (x < b && b <= y)
	|| (a <= x && y <= b);
}

void init_mmio() {
  for(int i = 0; i < NR_REGION; i++) {
	for(int j = i + 1; j < NR_REGION; j++) {
	  assert(!region_collide(mmap_table[i].start, 
			mmap_table[i].end,
			mmap_table[j].start,
			mmap_table[j].end
			));
	}
  }
}
