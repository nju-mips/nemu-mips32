#include "common.h"
#include "utils/elfsym.h"

typedef struct {
  vaddr_t pc;
  uint32_t instr;
  bool instr_enq;
} iq_element_t;

static int pc_ptr = 0, instr_ptr = 0;
static iq_element_t iq[20];

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
#if !CONFIG_INSTR_LOG
  panic("CONFIG_INSTR_LOG is needed for get_current_pc\n");
#endif
  return iq[instr_ptr].pc;
}

uint32_t get_current_instr() {
#if !CONFIG_INSTR_LOG
  panic(
      "CONFIG_INSTR_LOG is needed for get_current_instr\n");
#endif
  if (iq[instr_ptr].instr_enq) return iq[instr_ptr].instr;
  return 0;
}

void kdbg_print_instr_queue(void) {
  int i = pc_ptr;
  do {
    if (iq[i].instr_enq)
      eprintf("0x%08x: %08x %s\n", iq[i].pc, iq[i].instr,
          elfsym_find_symbol(&elfsym, iq[i].pc));
    else
      eprintf("0x%08x: xxxxxxxx %s\n", iq[i].pc,
          elfsym_find_symbol(&elfsym, iq[i].pc));
    i = (i + 1) % NR_IQ;
  } while (i != pc_ptr);
}
