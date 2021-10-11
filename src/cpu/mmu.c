#include "common.h"
#include "cpu/reg.h"
#include "device.h"
#include "cpu/memory.h"
#include "cpu/mmu.h"

extern device_t blackhole_dev;

tlb_entry_t tlb[NR_TLB_ENTRY];

extern void raise_exception(unsigned);

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
  uint32_t mask = cpu.cp0.pagemask.mask;
  uint32_t vpn = (cpu.cp0.entry_hi.vpn & ~mask) << 13;
  uint32_t pfn0 = (cpu.cp0.entry_lo0.pfn & ~mask) << 12;
  uint32_t pfn1 = (cpu.cp0.entry_lo1.pfn & ~mask) << 12;
  eprintf("%08x: TLB[%d]: MAP %08x -> %08x, %08x -> %08x\n", cpu.pc, i, vpn,
      pfn0, vpn | 0x1000, pfn1);
#endif
  tlb[i].pagemask = cpu.cp0.pagemask.mask;
  tlb[i].vpn = cpu.cp0.entry_hi.vpn & ~cpu.cp0.pagemask.mask;
  tlb[i].asid = cpu.cp0.entry_hi.asid;

  tlb[i].g = cpu.cp0.entry_lo0.g & cpu.cp0.entry_lo1.g;

  tlb[i].p0.pfn = cpu.cp0.entry_lo0.pfn & ~cpu.cp0.pagemask.mask;
  tlb[i].p0.c = cpu.cp0.entry_lo0.c;
  tlb[i].p0.d = cpu.cp0.entry_lo0.d;
  tlb[i].p0.v = cpu.cp0.entry_lo0.v;

  // tlb[i].p1.pfn = cpu.cp0.entry_lo1.pfn & ~cpu.cp0.pagemask.mask;
  tlb[i].p1.c = cpu.cp0.entry_lo1.c;
  tlb[i].p1.d = cpu.cp0.entry_lo1.d;
  tlb[i].p1.v = cpu.cp0.entry_lo1.v;
}

static void tlb_exception(int ex, int code, vaddr_t vaddr, unsigned asid) {
  cpu.cp0.badvaddr = vaddr;
  cpu.cp0.context.BadVPN2 = vaddr >> 13;
  cpu.cp0.entry_hi.vpn = vaddr >> 13;
  cpu.cp0.entry_hi.asid = asid;
  raise_exception(MAKE_EX(ex, code));
#if 0
  eprintf("%08x: TLB ex %d, code %d, vaddr %08x, ERL %d\n", cpu.pc, ex, code,
      vaddr, cpu.cp0.status.ERL);
#endif
}

vaddr_t page_translate(vaddr_t vaddr, mmu_attr_t *attr) {
  uint32_t exccode = attr->rwbit == MMU_LOAD ? EXC_TLBL : EXC_TLBS;
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
      if (attr->exbit)
        tlb_exception(EX_TLB_INVALID, exccode, vaddr, tlb[i].asid);
      return blackhole_dev.start;
    } else if (phyn->d == 0 && attr->rwbit == MMU_STORE) {
      if (attr->exbit)
        tlb_exception(EX_TLB_MODIFIED, EXC_TLBM, vaddr, tlb[i].asid);
      attr->dirty = phyn->d;
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
      uint32_t mask = tlb[i].pagemask;
      uint32_t vpn = (tlb[i].vpn & ~mask) << 13;
      uint32_t pfn0 = (tlb[i].p0.pfn & ~mask) << 12;
      uint32_t pfn1 = (tlb[i].p1.pfn & ~mask) << 12;
      eprintf("%08x: TLB[%d]: %08x -> %08x, %08x -> %08x, R: %08x -> %08x\n",
          cpu.pc, i, vpn, pfn0, vpn | 0x1000, pfn1, vaddr, highbits | lowbits);
#endif
      attr->dirty = phyn->d;
      return highbits | lowbits;
    }
  }

  if (attr->exbit)
    tlb_exception(EX_TLB_REFILL, exccode, vaddr, cpu.cp0.entry_hi.asid);
  attr->miss = 1;
  return blackhole_dev.start;
}
