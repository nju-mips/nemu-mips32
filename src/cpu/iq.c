#include "nemu.h"
#include "monitor.h"
#include "memory.h"

typedef struct {
  vaddr_t pc;
  uint32_t instr;
  bool instr_enq;
} iq_element_t;

static int pc_ptr = 0, instr_ptr = 0;
static iq_element_t iq[8];

#define NR_IQ (sizeof(iq) / sizeof(iq[0]))

void instr_enqueue_pc(vaddr_t pc) {
  instr_ptr = pc_ptr;
  iq[pc_ptr].pc = pc;
  iq[pc_ptr].instr_enq = false;
  pc_ptr = (pc_ptr + 1) % NR_IQ;
}

void instr_enqueue_instr(uint32_t instr) {
  iq[instr_ptr].instr_enq = true;
  iq[instr_ptr].instr = instr;
}

uint32_t get_current_pc() {
#ifndef DEBUG
  panic("please enable DEBUG macro in Makefile\n");
#endif
  return iq[instr_ptr].pc;
}

uint32_t get_current_instr() {
#ifndef ENABLE_INSTR_LOG
  panic("please enable ENABLE_INSTR_LOG macro in Makefile\n");
#endif
  if(iq[instr_ptr].instr_enq)
    return iq[instr_ptr].instr;
  return 0;
}

void print_instr_queue(void) {
  eprintf("last executed %ld instrs:\n", NR_IQ);
  int i = pc_ptr;
  do {
	if(iq[i].instr_enq)
	  eprintf("0x%08x: %08x\n", iq[i].pc, iq[i].instr);
	else
	  eprintf("0x%08x: xxxxxxxx\n", iq[i].pc);
	i = (i + 1) % NR_IQ;
  } while(i != pc_ptr);
}
