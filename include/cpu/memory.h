#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"
#include "cpu/reg.h"
#include "cpu/mmu.h"

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

/* for npc diff */
uint32_t paddr_peek(paddr_t, int);

/* for gdb debugger */
uint32_t dbg_vaddr_read(vaddr_t, int);
void dbg_vaddr_write(vaddr_t addr, int len, uint32_t data);

static inline vaddr_t ioremap(vaddr_t vaddr) { return vaddr & 0x1FFFFFFF; }

enum { MMU_LOAD, MMU_STORE };

static inline vaddr_t prot_addr_with_attr(vaddr_t addr, mmu_attr_t *attr) {
  switch ((addr >> 29) & 0x7) {
  case 4: /* kseg0 */
  case 5: /* kseg1 */
    return ioremap(addr);
  case 7: /* kseg3 */
    panic("%08x: addr %08x is invalid\n", cpu.pc, addr);
  case 6: /* supervisor */
    if (!CONFIG_IS_ENABLED(PAGING)) {
      return addr;
    } else {
      vaddr_t paddr = page_translate(addr, attr);
      return ioremap(paddr);
    }
  case 0 ... 3: /* kuseg */
    if (!CONFIG_IS_ENABLED(PAGING) || cpu.cp0.status.ERL) {
      return addr;
    } else {
      vaddr_t paddr = page_translate(addr, attr);
      return ioremap(paddr);
    }
  }
  assert (0);
}

static inline vaddr_t prot_addr(vaddr_t addr, bool rwbit) {
  mmu_attr_t attr = {.rwbit = rwbit, .exbit = 1};
  return prot_addr_with_attr(addr, &attr);
}

#endif
