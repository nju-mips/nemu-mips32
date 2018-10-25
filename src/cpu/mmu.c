#include "cpu/core.h"
#include "cpu/mmu.h"
#include "common.h"
#include "memory.h"
#include "device.h"

tlb_entry_t tlb[NR_TLB_ENTRY];

extern void signal_exception(int);

#define PAGE_MASK ((1 << 12) - 1)
#define VPN_MASK ((1 << 19) - 1)

static inline bool match_vpn_and_asid(int idx, uint32_t vpn, uint32_t asid) {
  bool vpn_match = (tlb[idx].vpn & VPN_MASK) == (vpn & VPN_MASK);
  bool asid_match = tlb[idx].asid == asid;
  return (vpn_match && (asid_match || tlb[idx].g));
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
  cpu.cp0.pagemask.mask = tlb[i].pagemask;
  cpu.cp0.entry_hi.vpn = tlb[i].vpn & ~tlb[i].pagemask;
  cpu.cp0.entry_hi.asid = tlb[i].asid;

  cpu.cp0.entry_lo0.pfn = tlb[i].p0.pfn & ~tlb[i].pagemask;
  cpu.cp0.entry_lo0.c = tlb[i].p0.c;
  cpu.cp0.entry_lo0.d = tlb[i].p0.d;
  cpu.cp0.entry_lo0.v = tlb[i].p0.v;
  cpu.cp0.entry_lo0.g = tlb[i].g;

  cpu.cp0.entry_lo1.pfn = tlb[i].p1.pfn & ~tlb[i].pagemask;
  cpu.cp0.entry_lo1.c = tlb[i].p1.c;
  cpu.cp0.entry_lo1.d = tlb[i].p1.d;
  cpu.cp0.entry_lo1.v = tlb[i].p1.v;
  cpu.cp0.entry_lo1.g = tlb[i].g;
}

void tlb_write(uint32_t i) {
#if 0
  printf("[NEMU] map 0x282b990:%08x v:[%08x, %08x] to p:[%08x, %08x], v:[%d %d], d:[%d, %d], g:[%d, %d]\n",
	  paddr_peek(0x282b990, 4),
	  cpu.cp0.entry_hi.vpn << 13,
	  (cpu.cp0.entry_hi.vpn << 13) + (4 * 1024),
	  cpu.cp0.entry_lo0.pfn << 12,
	  cpu.cp0.entry_lo1.pfn << 12,
	  cpu.cp0.entry_lo0.v, cpu.cp0.entry_lo1.v,
	  cpu.cp0.entry_lo0.d, cpu.cp0.entry_lo1.d,
	  cpu.cp0.entry_lo0.g, cpu.cp0.entry_lo1.g
  );
#endif
  tlb[i].pagemask = cpu.cp0.pagemask.mask;
  tlb[i].vpn = cpu.cp0.entry_hi.vpn & ~cpu.cp0.pagemask.mask;
  tlb[i].asid = cpu.cp0.entry_hi.asid;

  tlb[i].g = cpu.cp0.entry_lo0.g & cpu.cp0.entry_lo1.g;

  tlb[i].p0.pfn = cpu.cp0.entry_lo0.pfn & ~cpu.cp0.pagemask.mask;
  tlb[i].p0.c = cpu.cp0.entry_lo0.c;
  tlb[i].p0.d = cpu.cp0.entry_lo0.d;
  tlb[i].p0.v = cpu.cp0.entry_lo0.v;

  tlb[i].p1.pfn = cpu.cp0.entry_lo1.pfn & ~cpu.cp0.pagemask.mask;
  tlb[i].p1.c = cpu.cp0.entry_lo1.c;
  tlb[i].p1.d = cpu.cp0.entry_lo1.d;
  tlb[i].p1.v = cpu.cp0.entry_lo1.v;
}

paddr_t page_translate(vaddr_t vaddr, prot_info_t *prot) {
#define PROT_EXCEPT(__excode) ({   \
  if(!prot->igbit) {               \
	signal_exception(__excode);    \
	cpu.cp0.badvaddr = vaddr;      \
  }                                \
  prot->exbit = 1;                 \
  BADP_ADDR;                       \
})

  uint32_t excode = prot->rwbit == MMU_LOAD ? EXC_TLBL : EXC_TLBS;
  vaddr_mapped_t *mapped = (vaddr_mapped_t *)&vaddr;
  for(int i = 0; i < NR_TLB_ENTRY; i++) {
	if(!match_vpn_and_asid(i, mapped->vpn, cpu.cp0.entry_hi.asid)) {
	  continue;
	}

	/* match the vpn and asid */
	tlb_phyn_t *phyn = mapped->oddbit ? &(tlb[i].p1) : &(tlb[i].p0);
	if(phyn->v == 0) {
	  // printf("[TLB@%08x] invalid phyn, signal(%d)\n", cpu.pc, excode);
	  return PROT_EXCEPT(excode);
	} else if(prot->rwbit == MMU_STORE && phyn->d == 0) {
	  // printf("[TLB@%08x] modified phyn, signal(%d)\n", cpu.pc, excode);
	  return PROT_EXCEPT(EXC_TLBM);
	} else {
	  // printf("[TLB@%08x] matched %08x -> %08x\n", cpu.pc, vaddr, (phyn->pfn << 12) | (vaddr & PAGE_MASK));
	  return (phyn->pfn << 12) | (vaddr & PAGE_MASK);
	}
  }

  // printf("[TLB@%08x] unmatched %08x\n", cpu.pc, vaddr);
  return PROT_EXCEPT(excode);
#undef PROT_EXCEPT
}
