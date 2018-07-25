#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "common.h"
#include "cpu/reg.h"

extern uint8_t ddr[];
uint32_t vaddr_read(vaddr_t, int);
uint32_t paddr_read(paddr_t, int);
void vaddr_write(vaddr_t, int, uint32_t);

uint32_t vaddr_read_safe(vaddr_t, int);
void vaddr_write_safe(vaddr_t addr, int len, uint32_t data);

static inline vaddr_t prot_addr(uint32_t addr) {
#ifdef ENABLE_SEGMENT
  return addr + cpu.base;
#elif defined ENABLE_PAGING
  vaddr_t page_translate(vaddr_t);
  return page_translate(addr);
#else
  return addr;
#endif
}

#endif
