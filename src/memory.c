#include <stdlib.h>
#include <SDL/SDL.h>
#include "nemu.h"
#include "monitor.h"
#include "device.h"
#include "memory.h"


uint32_t badp_read(paddr_t addr, int len) {
  return 0;
}

void badp_write(paddr_t addr, int len, uint32_t data) {
  return;
}

// the memory mapping of mips32-npc
struct mmap_region {
  uint32_t start, end;
  read_func read;
  write_func write;
  map_func map;
  peek_func peek;
} mmap_table [] = {
/*  start             end                   reader         writer      mapper    peeker   */
  {BADP_ADDR,   BADP_ADDR + BADP_SIZE,     badp_read,    badp_write,   NULL,     badp_read   },
  {DDR_BASE,    DDR_BASE + DDR_SIZE,       ddr_read,     ddr_write,    ddr_map,  ddr_read    },
  {BRAM_BASE,   BRAM_BASE + BRAM_SIZE,     bram_read,    bram_write,   bram_map, bram_read   },
  {GPIO_BASE,   GPIO_BASE + GPIO_SIZE,     invalid_read, gpio_write,   NULL,     invalid_read},
  {SERIAL_ADDR, SERIAL_ADDR + SERIAL_SIZE, serial_read,  serial_write, NULL,     serial_peek },
  {MAC_ADDR,    MAC_ADDR + MAC_SIZE,       mac_read,     mac_write,    NULL,     mac_read    },
  // {KB_ADDR, KB_ADDR + KB_SIZE, kb_read, invalid_write},
  // {VGA_BASE, VGA_BASE + VGA_SIZE, vga_read, vga_write},
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

void *paddr_map(uint32_t addr, uint32_t len) {
  // only unmapped address can be map
  Assert(is_unmapped(addr), "addr %08x should be unmapped\n", addr);

  int idx = find_region(ioremap(addr));
  Assert(idx != -1, "address(0x%08x) is out of bound, pc(0x%08x)\n", addr, cpu.pc);
  Assert(mmap_table[idx].map, "cannot find map handler for address(0x%08x), pc(0x%08x)\n", addr, cpu.pc);
  return mmap_table[idx].map(ioremap(addr) - mmap_table[idx].start, len);
}

uint32_t vaddr_read_safe(vaddr_t addr, int len) {
  addr = prot_addr(addr, MMU_LOAD);
  int idx = find_region(addr);
  if(idx == -1) return 0;
  return mmap_table[idx].read(addr - mmap_table[idx].start, len);
}

void vaddr_write_safe(vaddr_t addr, int len, uint32_t data) {
  addr = prot_addr(addr, MMU_STORE);
  int idx = find_region(addr);
  if(idx == -1) return;
  return mmap_table[idx].write(addr - mmap_table[idx].start, len, data);
}

uint32_t vaddr_read(vaddr_t addr, int len) {
  addr = prot_addr(addr, MMU_LOAD);
  int idx = find_region(addr);
  CPUAssert(idx != -1, "address(0x%08x:0x%08x) is out of bound, pc(0x%08x)\n", addr, addr, cpu.pc);
  return mmap_table[idx].read(addr - mmap_table[idx].start, len);
}

uint32_t vaddr_peek(vaddr_t addr, int len) {
  addr = prot_addr(addr, MMU_LOAD);
  int idx = find_region(addr);
  CPUAssert(idx != -1, "address(0x%08x:0x%08x) is out of bound, pc(0x%08x)\n", addr, addr, cpu.pc);
  return mmap_table[idx].peek(addr - mmap_table[idx].start, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  addr = prot_addr(addr, MMU_STORE);
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
