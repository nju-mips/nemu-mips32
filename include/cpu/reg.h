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

#define CP0_INDEX        0
#define CP0_RANDOM       1
#define CP0_ENTRY_LO0    2
#define CP0_ENTRY_LO1    3
#define CP0_PAGE_CONTEXT 4  // maintained by kernel
#define CP0_PAGE_MASK    5
#define CP0_WIRED        6
#define CP0_RESERVED     7  // for extra debug and segment
#define CP0_BADVADDR     8
#define CP0_COUNT        9
#define CP0_ENTRY_HI     10
#define CP0_COMPARE      11
#define CP0_STATUS       12
#define CP0_CAUSE        13
#define CP0_EPC          14
#define CP0_PRID         15
#define CP0_CONFIG       16

#define CP0_TAG_LO       28
#define CP0_TAG_HI       29

/*
 * default config:
 *   {1'b1, 21'b0, 3'b1, 4'b0, cp0_regs_Config[2:0]}; //Release 1
 *
 * default config1:
 *   {1'b0, 6'd15, 3'd1, 3'd5, 3'd0, 3'd2, 3'd5, 3'd0, 7'd0}; 
 *
 *   Cache Size:   I:128-64B-direct, D:256-64B-direct
 */

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

typedef struct {
  uint32_t revision        : 8;
  uint32_t processor_id    : 8;
  uint32_t company_id      : 8;
  uint32_t company_options : 8;
} cp0_prid_t;


typedef struct {
  uint32_t K0   : 3; // kseg0 coherency algorithms
  uint32_t _0   : 4; // must be zero
  uint32_t MT   : 3; // MMU type
                     // 0 for none
					 // 1 for standard TLB
					 // 2 xxx, 3 xxx
  uint32_t AR   : 3; // 0 for revision 1
  uint32_t AT   : 2; // 0 for mips32,
                     // 1 for mips64 with access only to 32-bit seg
                     // 2 for mips64 with all access to 32-bit seg
					 // 3 reserved
  uint32_t BE   : 1; // 0 for little endian, 1 for big endian
  uint32_t Impl : 15;
  uint32_t M    : 1; // donate that config1 impled at sel 1
} cp0_config_t;

// 1'b0, 6'd15, 3'd1, 3'd5, 3'd0, 3'd2, 3'd5, 3'd0, 7'd0
typedef struct {
  uint32_t FP : 1; // FPU present bit
  uint32_t EP : 1; // EJTAG present bit
  uint32_t CA : 1; // code compression present bit
  uint32_t WR : 1; // watch registers present bit

  uint32_t PC : 1; // performance counter present bit
  uint32_t MD : 1; // not used on mips32 processor
  uint32_t C2 : 1; // coprocessor present bit
  
  uint32_t DA : 3; // dcache associativity
                   // 0 for direct mapped
				   // 2^(DA) ways
				   // ---------------------------
  uint32_t DL : 3; // dcache line size: 
                   // 0 for no icache, 7 reserved
				   // othwise: 2^(DL + 1) bytes
				   // ---------------------------
  uint32_t DS : 3; // dcache sets per way:
                   // 2^(IS + 8)
				   // ---------------------------
  uint32_t IA : 3; // icache associativity
                   // 0 for direct mapped
				   // 2^(IA) ways
				   // ---------------------------
  uint32_t IL : 3; // icache line size: 
                   // 0 for no icache, 7 reserved
				   // othwise: 2^(IL + 1) bytes
				   // ---------------------------
  uint32_t IS : 3; // icache sets per way:
                   // 2^(IS + 8)
				   // ---------------------------
  uint32_t MMU_size : 6; // 0 to 63 indicates 1 to 64 TLB entries
  uint32_t M  : 1; // indicate config 2 is present
} cp0_config1_t;

typedef struct {
  uint32_t vpn2 : 19;
  uint32_t _0   : 1;
  uint32_t asid : 8;
} cp0_entry_hi_t;

typedef struct {
  uint32_t _0  : 2;
  uint32_t pfn : 25;
  uint32_t c   : 2;
  uint32_t d   : 1;
  uint32_t v   : 1;
  uint32_t g   : 1;
} cp0_entry_lo_t;

typedef struct {
  uint32_t _0 : 2;
  uint32_t mask : 16;
} cp0_page_mask_t;

typedef struct {
} cp0_wired_t;


#define CAUSE_IP_TIMER 0x80

#define EXC_INTR    0
#define EXC_TLBM    1  /* tlb modification */
#define EXC_TLBL    2  /* tlb load */
#define EXC_TLBS    3  /* tlb store */
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
_Static_assert(sizeof(cp0_prid_t) == sizeof(cpu.cp0[CP0_PRID][0]), "assertion of sizeof(cp0_prid_t) failed");
_Static_assert(sizeof(cp0_config_t) == sizeof(cpu.cp0[CP0_CONFIG][0]), "assertion of sizeof(cp0_config_t) failed");
_Static_assert(sizeof(cp0_config1_t) == sizeof(cpu.cp0[CP0_CONFIG][1]), "assertion of sizeof(cp0_config1_t) failed");

#endif
