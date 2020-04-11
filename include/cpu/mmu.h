#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>

#include "cpu/reg.h"

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

void tlb_present();
void tlb_read(uint32_t i);
void tlb_write(uint32_t i);

typedef struct {
  /* as argument */
  uint32_t rwbit : 1;
  uint32_t exbit : 1;
  /* as return value */
  uint32_t dirty : 1;
  uint32_t miss  : 1;
} mmu_attr_t;

vaddr_t page_translate(vaddr_t, mmu_attr_t *attr);

#endif
