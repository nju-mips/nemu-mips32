#include "mmu.h"
#include "cpu/reg.h"
#include "common.h"

tlb_entry_t tlb_entries[NR_TLB_ENTRY];

extern void signal_exception(int);

#define EXC_TLB_MISS 0
#define EXC_TLB_MODIFIED 1
#define EXC_TLB_INVALID 2

static inline bool match_vpn_and_asid(int idx, uint32_t vpn, uint32_t asid) {
  uint32_t not_pagemask = ~(tlb_entries[idx].pagemask);
  bool vpn_match = (tlb_entries[idx].vpn & not_pagemask) == (vpn & not_pagemask);
  bool asid_match = tlb_entries[idx].asid == asid;
  if(! (vpn_match && asid_match && tlb_entries[idx].g) ) {
	return false;
  }
  return true;
}

void tlb_present() {
  for(int i = 0; i < NR_TLB_ENTRY; i++) {
	if(!match_vpn_and_asid(i, cpu.cp0.entry_hi.vpn, cpu.cp0.entry_hi.asid)) {
	  continue;
	}
	/* match this tlb entry */
	cpu.cp0.index.p = 0;
	cpu.cp0.index.idx = i;
	return;
  }
  cpu.cp0.index.p = 1;
}

void tlb_read(Inst inst) {
}

void tlb_write_by_index(Inst inst) {
}

void tlb_write_randomly(Inst inst) {
}

vaddr_t page_translate(vaddr_t vaddr) {
  vaddr_mapped_t *mapped = (vaddr_mapped_t *)&vaddr;
  for(int i = 0; i < NR_TLB_ENTRY; i++) {
	uint32_t not_pagemask = ~(tlb_entries[i].pagemask);
	if(!match_vpn_and_asid(i, mapped->vpn, cpu.cp0.entry_hi.asid)) {
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
