#ifndef CPU_MMU_H
#define CPU_MMU_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu/core.h"

#define NR_TLB_ENTRY 512

typedef struct {
  uint32_t pfn : 24;
  uint32_t c   : 3;  // cache coherency
  uint32_t d   : 1;  // dirty
  uint32_t v   : 1;  // valid
} tlb_phyn_t;

typedef struct {
  uint16_t pagemask;

  struct {
	uint32_t vpn : 19;
	uint32_t g    : 1;
	uint32_t asid : 8;
  };

  tlb_phyn_t p0;
  tlb_phyn_t p1;
} tlb_entry_t;

typedef union {
  struct {
	uint32_t off    : 12;
	uint32_t oddbit : 1;
	uint32_t vpn    : 19;
  };
  uint32_t val;
} vaddr_mapped_t;

typedef union {
#define MMU_LOAD  0
#define MMU_STORE 1
  struct {
	int exbit : 1; /* indicate whether exception triggered*/
	int igbit : 1; /* ignore exception: default 0 */
	int rwbit : 1;
  };
  int val;
} prot_info_t;

void tlb_present();
void tlb_read(uint32_t i);
void tlb_write(uint32_t i);

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

typedef struct {
  void *p;
  size_t size;
} map_result_t;

/* for nemu core */
uint32_t vaddr_read(vaddr_t, int);
void vaddr_write(vaddr_t, int, uint32_t);

/* for npc diff */
uint32_t paddr_peek(paddr_t, int);

vaddr_t page_translate(vaddr_t, prot_info_t *);

static inline vaddr_t iomap(vaddr_t vaddr) {
  return vaddr & 0x1FFFFFFF;
}

static inline vaddr_t prot_addr(vaddr_t addr, prot_info_t *prot) {
#ifdef ENABLE_SEGMENT
  addr += cpu.base;
#endif
  if(is_unmapped(addr)) {
	//  0x80000000 -> 0x00000000
	//  0x90000000 -> 0x10000000
	//  0xA0000000 -> 0x00000000
	//  0xB0000000 -> 0x10000000
	return iomap(addr);
  } else {
#if defined ENABLE_PAGING
	return page_translate(addr, prot);
#else
	return addr;
#endif
  }
}

/* read/write/exception */
static inline vaddr_t prot_addr_rw_except(vaddr_t addr, bool rwbit) {
  prot_info_t prot = {0};
  prot.rwbit = rwbit;
  return prot_addr(addr, &prot);
}

/* read/write/noexcept */
static inline vaddr_t prot_addr_rw_noexcept(vaddr_t addr, bool rwbit) {
  prot_info_t prot = {0};
  prot.rwbit = rwbit;
  prot.igbit = 1;
  return prot_addr(addr, &prot);
}

#endif
