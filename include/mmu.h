#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stdbool.h>
#include "cpu/reg.h"

#define NR_TLB_ENTRY 512

typedef struct {
  uint32_t pfn : 25;
  uint32_t c   : 2;  // cache coherency
  uint32_t d   : 2;  // dirty
  uint32_t v   : 2;  // valid
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
	uint32_t vpn : 19;
	uint32_t oe  : 1;
	uint32_t off : 12;
  };
  uint32_t val;
} vaddr_mapped_t;


void tlb_present();
void tlb_read(uint32_t i);
void tlb_write(uint32_t i);


#endif
