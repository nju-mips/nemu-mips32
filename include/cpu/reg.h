#ifndef __REG_H__
#define __REG_H__

#include "common.h"

typedef struct {
  uint32_t gpr[32];
  uint32_t hi, lo;
  vaddr_t pc;
} CPU_state;

typedef struct {
	union {
		uint32_t val;
		// R-type
		struct {
			uint32_t func  :6;
			uint32_t shamt :5;
			uint32_t rd    :5;
			uint32_t rt    :5;
			uint32_t rs    :5;
			uint32_t op    :6;
		};

		// I-type
		struct {
			uint32_t uimm   :16;
		};

		// SI-type
		struct {
			int32_t simm :16;
		};

		// J-type
		struct {
			uint32_t addr  :26;
		};

		// unaligned-iDX
		struct {
			uint32_t idx:2;
		};
	};
} Inst; // Instruction

extern const char *regs[];
extern CPU_state cpu;
#endif
