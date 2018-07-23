#ifndef __REG_H__
#define __REG_H__

#include "common.h"

typedef struct {
  uint32_t gpr[32];
  uint32_t cp0[32][8];
  uint32_t hi, lo;
  vaddr_t pc;
#ifdef ENABLE_SEGMENT
  vaddr_t base;
#endif
} CPU_state;

#ifdef ENABLE_SEGMENT
#define CP0_RESERVED_BASE 0   // for segment
#endif

#define CP0_RESERVED_SERIAL 1

#define CP0_RESERVED 7  // for extra debug
#define CP0_BADVADDR 8
#define CP0_COUNT    9
#define CP0_COMPARE  11
#define CP0_STATUS   12
#define CP0_CAUSE    13
#define CP0_EPC      14

typedef struct {
	uint32_t IE   : 1;
	uint32_t EXL  : 1;
	uint32_t ERL  : 1;
	uint32_t R0   : 1;

	uint32_t UM   : 1;
	uint32_t UX   : 1;
	uint32_t SX   : 1;
	uint32_t KX   : 1;

	uint32_t IM   : 8;

	uint32_t Impl : 2;
	uint32_t _0   : 1;
	uint32_t NMI  : 1;
	uint32_t SR   : 1;
	uint32_t TS   : 1;

	uint32_t BEV  : 1;
	uint32_t PX   : 1;

	uint32_t MX   : 1;
	uint32_t RE   : 1;
	uint32_t FR   : 1;
	uint32_t RP   : 1;
	uint32_t CU   : 4;
} cp0_status_t;

typedef struct {
	uint32_t _5 : 2;
	uint32_t ExcCode : 5;
	uint32_t _4 : 1;
	uint32_t IP : 8;

	uint32_t _3 : 6;
	uint32_t WP : 1;
	uint32_t IV : 1;

	uint32_t _2 : 4;
	uint32_t CE : 2;
	uint32_t _1 : 1;
	uint32_t BD : 1;
} cp0_cause_t;

#define CAUSE_IP_TIMER 0x80

#define EXC_INTR    0
#define EXC_TLB     1
#define EXC_TLBL    2
#define EXC_TLBS    3
#define EXC_AdEL    4
#define EXC_AdES    5
#define EXC_IBE     6
#define EXC_DBE     7
#define EXC_SYSCALL 8
#define EXC_BP      9
#define EXC_RI      10
#define EXC_CPU     11
#define EXC_OV      12
#define EXC_TRAP    13

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

		// MFC0
		struct {
			uint32_t sel:3;
		};
	};
} Inst; // Instruction

extern CPU_state cpu;
int init_cpu(vaddr_t entry);

_Static_assert(sizeof(Inst) == sizeof(uint32_t), "assertion of sizeof(Inst) failed");
_Static_assert(sizeof(cp0_status_t) == sizeof(cpu.cp0[CP0_STATUS][0]), "assertion of sizeof(cp0_status_t) failed");
_Static_assert(sizeof(cp0_cause_t) == sizeof(cpu.cp0[CP0_CAUSE][0]), "assertion of sizeof(cp0_cause_t) failed");

#endif
