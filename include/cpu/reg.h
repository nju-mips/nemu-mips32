#ifndef __REG_H__
#define __REG_H__

#include "common.h"

typedef struct {
  uint32_t gpr[32];
  uint32_t hi, lo;
  vaddr_t pc;
} CPU_state;

extern const char *regs[];
extern CPU_state cpu;
#endif
