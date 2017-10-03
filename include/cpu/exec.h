#ifndef __CPU_EXEC_H__
#define __CPU_EXEC_H__

#include "nemu.h"

static inline uint32_t instr_fetch(vaddr_t *pc, int len) {
  uint32_t instr = vaddr_read(*pc, len);
  (*pc) += len;
  return instr;
}

#endif
