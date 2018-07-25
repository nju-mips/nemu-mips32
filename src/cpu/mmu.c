#include "mmu.h"
#include "cpu/reg.h"
#include "common.h"

static tlb_entry_t tlb_entries[NR_TLB_ENTRY];

extern void signal_exception(int);

#define EXC_TLB_MISS 0
#define EXC_TLB_MODIFIED 1
#define EXC_TLB_INVALID 2

vaddr_t page_translate(vaddr_t vaddr) {
  vaddr_mapped_t *mapped = (vaddr_mapped_t *)&vaddr;
  cp0_entry_hi_t *entry_hi = (cp0_entry_hi_t *)&(cpu.cp0[CP0_ENTRY_HI][0]);
  for(int i = 0; i < NR_TLB_ENTRY; i++) {
	uint32_t not_pagemask = ~(tlb_entries[i].pagemask);
	bool vpn_match = (tlb_entries[i].vpn2 & not_pagemask) ==
	  (mapped->vpn & not_pagemask);
	bool asid_match = tlb_entries[i].asid == entry_hi->asid;
	if(! (vpn_match && asid_match && tlb_entries[i].g) ) {
	  continue;
	}

	/* match the vpn and asid */
	tlb_phyn_t *phyn = (mapped->oe == 0) ? &(tlb_entries[i].p0) : &(tlb_entries[i].p1);
	if(phyn->v == 0) {
	  signal_exception(EXC_TLB_INVALID);
	  return -1;
	} else if(phyn->d == 0 && false) {
	  signal_exception(EXC_TLB_MODIFIED);
	  return -1;
	} else {
	  uint32_t mask = not_pagemask << 12;
	  return ((phyn->pfn << 12) & mask) || (vaddr & ~mask);
	}
  }

  signal_exception(EXC_TLB_MISS);
  return -1;
}
