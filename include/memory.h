#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"
#include "cpu/reg.h"

/* 
 * FFFF FFFF -\
 *            +----- kernel mapped
 * E000 0000 -/
 * DFFF FFFF -\
 *            +----- supervisor mapped
 * C000 0000 -/
 * BFFF FFFF -\
 *            +----- kernel unmapped uncached (kseg1)
 * A000 0000 -/
 * 9FFF FFFF -\
 *            +----- kernel unmapped (kseg0)
 * 8000 0000 -/
 * 7FFF FFFF -\
 *            |
 *            |
 *            |
 *            +----- user mapped
 *            |
 *            |
 *            |
 * 0000 0000 -/
 */

#define UNCACHED_BASE 0xA0000000
#define UNCACHED_END  0xC0000000

#define UNMAPPED_BASE 0x80000000
#define UNMAPPED_END  0xC0000000

static inline bool is_unmapped(uint32_t addr) {
  return UNMAPPED_BASE <= addr && addr < UNMAPPED_END;
}

static inline bool is_uncached(uint32_t addr) {
  return UNCACHED_BASE <= addr && addr < UNCACHED_END;
}

extern uint8_t ddr[];
extern uint8_t bram[];

/* for nemu core */
uint32_t vaddr_read(vaddr_t, int);
uint32_t vaddr_peek(paddr_t, int);
void vaddr_write(vaddr_t, int, uint32_t);

/* for gdb debugger */
uint32_t vaddr_read_safe(vaddr_t, int);
void vaddr_write_safe(vaddr_t addr, int len, uint32_t data);

vaddr_t page_translate(vaddr_t, bool rwbit);

static inline vaddr_t ioremap(vaddr_t vaddr) {
  return vaddr & 0x1FFFFFFF;
}

enum { MMU_LOAD, MMU_STORE };

static inline vaddr_t prot_addr(vaddr_t addr, bool rwbit) {
#ifdef ENABLE_SEGMENT
  return addr + cpu.base;
#elif defined ENABLE_PAGING
  if(is_unmapped(addr)) {
	//  0x80000000 -> 0x00000000
	//  0x90000000 -> 0x10000000
	//  0xA0000000 -> 0x00000000
	//  0xB0000000 -> 0x10000000
	return ioremap(addr);
  } else {
	return page_translate(addr, rwbit);
  }
#else
  return addr;
#endif
}

#endif
