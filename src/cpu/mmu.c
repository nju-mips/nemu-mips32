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

void tlb_read(uint32_t i) {
  cpu.cp0.pagemask = tlb_entries[i].pagemask;
  cpu.cp0.entry_hi.vpn = tlb_entries[i].vpn & ~tlb_entries[i].pagemask;
  cpu.cp0.entry_hi.asid = tlb_entries[i].asid;

  cpu.cp0.entry_lo0.pfn = tlb_entries[i].p0.pfn & ~tlb_entries[i].pagemask;
  cpu.cp0.entry_lo0.c = tlb_entries[i].p0.c;
  cpu.cp0.entry_lo0.d = tlb_entries[i].p0.d;
  cpu.cp0.entry_lo0.v = tlb_entries[i].p0.v;
  cpu.cp0.entry_lo0.g = tlb_entries[i].g;

  cpu.cp0.entry_lo1.pfn = tlb_entries[i].p1.pfn & ~tlb_entries[i].pagemask;
  cpu.cp0.entry_lo1.c = tlb_entries[i].p1.c;
  cpu.cp0.entry_lo1.d = tlb_entries[i].p1.d;
  cpu.cp0.entry_lo1.v = tlb_entries[i].p1.v;
  cpu.cp0.entry_lo1.g = tlb_entries[i].g;
}

void tlb_write(uint32_t i) {
  tlb_entries[i].pagemask = cpu.cp0.pagemask;
  tlb_entries[i].vpn = cpu.cp0.entry_hi.vpn & ~cpu.cp0.pagemask;
  tlb_entries[i].asid = cpu.cp0.entry_hi.asid;

  tlb_entries[i].g = cpu.cp0.entry_lo0.g & cpu.cp0.entry_lo1.g;

  tlb_entries[i].p0.pfn = cpu.cp0.entry_lo0.pfn & ~cpu.cp0.pagemask;
  tlb_entries[i].p0.c = cpu.cp0.entry_lo0.c;
  tlb_entries[i].p0.d = cpu.cp0.entry_lo0.d;
  tlb_entries[i].p0.v = cpu.cp0.entry_lo0.v;

  tlb_entries[i].p1.pfn = cpu.cp0.entry_lo1.pfn & ~cpu.cp0.pagemask;
  tlb_entries[i].p1.c = cpu.cp0.entry_lo1.c;
  tlb_entries[i].p1.d = cpu.cp0.entry_lo1.d;
  tlb_entries[i].p1.v = cpu.cp0.entry_lo1.v;
}

vaddr_t page_translate(vaddr_t vaddr) {
  CPUAssert(0, "fuckyou\n");
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
