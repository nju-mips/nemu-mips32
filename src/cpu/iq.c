#include "nemu.h"
#include "monitor.h"
#include "memory.h"

static int p = 0;
static vaddr_t iq[8];

#define NR_IQ (sizeof(iq) / sizeof(iq[0]))

void instr_enqueue(uint32_t pc) {
  iq[p] = pc;
  p = (p + 1) % NR_IQ;
}

void print_instr_queue(void) {
  eprintf("last executed %ld instrs:\n", NR_IQ);
  int i = p;
  do {
	if(is_unmapped(iq[i]))
	  eprintf("0x%08x: %08x\n", iq[i], vaddr_read_safe(iq[i], 4));
	else
	  eprintf("0x%08x: xxxxxxxx\n", iq[i]);
	i = (i + 1) % NR_IQ;
  } while(i != p);
}
