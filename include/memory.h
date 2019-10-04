#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"
#include "cpu.h"
#include "mmu.h"

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
#define UNCACHED_END 0xC0000000

#define UNMAPPED_BASE 0x80000000
#define UNMAPPED_END 0xC0000000

static inline bool is_unmapped(uint32_t addr) {
  return UNMAPPED_BASE <= addr && addr < UNMAPPED_END;
}

static inline bool is_uncached(uint32_t addr) {
  return UNCACHED_BASE <= addr && addr < UNCACHED_END;
}

typedef struct {
  void *p;
  size_t size;
} map_result_t;

/* for nemu core */
uint32_t vaddr_read(vaddr_t, int);
void vaddr_write(vaddr_t, int, uint32_t);

/* for npc diff */
uint32_t paddr_peek(paddr_t, int);

/* for gdb debugger */
uint32_t dbg_vaddr_read(vaddr_t, int);
void dbg_vaddr_write(vaddr_t addr, int len, uint32_t data);

static inline vaddr_t ioremap(vaddr_t vaddr) { return vaddr & 0x1FFFFFFF; }

enum { MMU_LOAD, MMU_STORE };

static inline vaddr_t prot_addr_with_attr(vaddr_t addr, mmu_attr_t attr) {
#ifdef CONFIG_SEGMENT
  addr += cpu.base;
#endif
  if (is_unmapped(addr)) {
    //  0x80000000 -> 0x00000000
    //  0x90000000 -> 0x10000000
    //  0xA0000000 -> 0x00000000
    //  0xB0000000 -> 0x10000000
    return ioremap(addr);
  } else {
#if defined CONFIG_PAGING
    vaddr_t paddr = page_translate(addr, attr);
    return ioremap(paddr);
#else
    return addr;
#endif
  }
}

static inline vaddr_t prot_addr(vaddr_t addr, bool rwbit) {
  mmu_attr_t attr = {.rwbit = rwbit, .exbit = 1};
  return prot_addr_with_attr(addr, attr);
}

#endif
