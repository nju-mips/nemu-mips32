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
 *            +----- kernel unmapped uncached
 * A000 0000 -/
 * 9FFF FFFF -\
 *            +----- kernel unmapped
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

#define UNMAPPED_BASE 0x80000000
#define UNMAPPED_END  0xC0000000
#define UNMAPPED_SIZE (4 * 1024 * 1024) /* only support 4 MB */

static inline bool is_unmapped(uint32_t addr) {
  return UNMAPPED_BASE <= addr && addr < UNMAPPED_END;
}

extern uint8_t ddr[];
uint32_t vaddr_read(vaddr_t, int);
uint32_t paddr_read(paddr_t, int);
void vaddr_write(vaddr_t, int, uint32_t);

uint32_t vaddr_read_safe(vaddr_t, int);
void vaddr_write_safe(vaddr_t addr, int len, uint32_t data);

vaddr_t page_translate(vaddr_t);

static inline vaddr_t prot_addr(uint32_t addr) {
#ifdef ENABLE_SEGMENT
  return addr + cpu.base;
#elif defined ENABLE_PAGING
  if(UNMAPPED_BASE <= addr && addr < UNMAPPED_END) {
	return addr;
  } else {
	return page_translate(addr);
  }
#else
  return addr;
#endif
}

#endif
