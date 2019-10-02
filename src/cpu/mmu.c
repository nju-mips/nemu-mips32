#include "common.h"
#include "cpu/reg.h"
#include "device.h"
#include "memory.h"
#include "mmu.h"

tlb_entry_t tlb[NR_TLB_ENTRY];

extern void signal_exception(unsigned);

void tlb_present() {
  for (int i = 0; i < NR_TLB_ENTRY; i++) {
    uint32_t mask = tlb[i].pagemask;
    assert(mask == 0 || (mask | (mask + 1)) == 0);
    bool match = (tlb[i].vpn & ~mask) == (cpu.cp0.entry_hi.vpn & ~mask) &&
                 (tlb[i].g || tlb[i].asid == cpu.cp0.entry_hi.asid);

    if (!match) continue;

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

static void tlb_exception(int ex, int code, vaddr_t vaddr, unsigned asid) {
  cpu.cp0.badvaddr = vaddr;
  cpu.cp0.context.BadVPN2 = vaddr >> 13;
  cpu.cp0.entry_hi.vpn = vaddr >> 13;
  cpu.cp0.entry_hi.asid = asid;
  signal_exception(MAKE_EX(ex, code));
}

vaddr_t page_translate(vaddr_t vaddr, mmu_attr_t attr) {
  uint32_t exccode = attr.rwbit == MMU_LOAD ? EXC_TLBL : EXC_TLBS;
  uint32_t va_31_13 = (vaddr & ~0x1FFF) >> 13;
  for (int i = 0; i < NR_TLB_ENTRY; i++) {
    uint32_t mask = tlb[i].pagemask;
    assert(mask == 0 || (mask | (mask + 1)) == 0);
    bool match = (tlb[i].vpn & ~mask) == (va_31_13 & ~mask) &&
                 (tlb[i].g || tlb[i].asid == cpu.cp0.entry_hi.asid);
    if (!match) continue;

    bool EvenOddBit = vaddr & ((mask + 1) << 12);
    /* match the vpn and asid */
    tlb_phyn_t *phyn = EvenOddBit ? &(tlb[i].p1) : &(tlb[i].p0);
    if (phyn->v == 0) {
#if 0
      printf("[TLB@%08x] invalid phyn, signal(%d)\n", cpu.pc, exccode);
#endif
      if (attr.exbit)
        tlb_exception(EX_TLB_INVALID, exccode, vaddr, tlb[i].asid);
      return blackhole_dev.start;
    } else if (phyn->d == 0 && attr.rwbit == MMU_STORE) {
#if 0
      printf("[TLB@%08x] modified phyn, signal(%d)\n", cpu.pc, exccode);
#endif
      if (attr.exbit)
        tlb_exception(EX_TLB_MODIFIED, EXC_TLBM, vaddr, tlb[i].asid);
      return blackhole_dev.start;
    } else {
      // # pfn_PABITS-1-12..0 corresponds to pa_PABITS-1..12
      //
      // pa ← pfn_PABITS-1-12..EvenOddBit-12 || va_EvenOddBit-1..0
      // case TLB[i].Mask
      //  2#0000000000000000: EvenOddBit ← 12
      //  2#0000000000000011: EvenOddBit ← 14
      //  2#0000000000001111: EvenOddBit ← 16
      // endcase

      uint32_t highbits = (phyn->pfn & ~mask) << 12;
      uint32_t lowbits = vaddr & (((mask + 1) << 12) - 1);
#if 0
      printf("[TLB@%08x] matched %08x -> %08x\n", cpu.pc, vaddr,
          highbits | lowbits);
#endif
      return highbits | lowbits;
    }
  }

#if 0
  printf("[TLB@%08x] TLBMiss %08x\n", cpu.pc, vaddr);
#endif
  if (attr.exbit)
    tlb_exception(EX_TLB_REFILL, exccode, vaddr, cpu.cp0.entry_hi.asid);
  return blackhole_dev.start;
}
