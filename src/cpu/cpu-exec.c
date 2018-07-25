#include <sys/time.h>
#include <setjmp.h>

#include "nemu.h"
#include "device.h"
#include "monitor.h"
#include "memory.h"
#include "mmu.h"

CPU_state cpu;

#ifdef ENABLE_PAGING
static jmp_buf cpu_exception_handler;
#endif

const char *regs[32] = {
  "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
};

#define LIKELY(cond) __builtin_expect(!!(cond), 1)

#define EXCEPTION_VECTOR_LOCATION 0x10000020
#define MAX_INSTR_TO_PRINT 10

nemu_state_t nemu_state = NEMU_STOP;

static uint64_t nemu_start_time = 0;

char asm_buf[80], *asm_buf_p;

// 1s = 10^3 ms = 10^6 us
static uint64_t get_current_time() { // in us
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec * 1000000 + t.tv_usec - nemu_start_time;
}

static int dsprintf(char *buf, const char *fmt, ...) {
#if 0
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
#endif
  return 0;
}


static uint32_t oldpc = 0;

void print_registers() {
  // print registers to stderr, so that will not mixed with uart output
  eprintf("$pc:    0x%08x    $hi:    0x%08x    $lo:    0x%08x\n", oldpc, cpu.hi, cpu.lo);
  eprintf("$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n", cpu.gpr[0], cpu.gpr[1], cpu.gpr[2], cpu.gpr[3]);
  eprintf("$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n", cpu.gpr[4], cpu.gpr[5], cpu.gpr[6], cpu.gpr[7]);
  eprintf("$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n", cpu.gpr[8], cpu.gpr[9], cpu.gpr[10], cpu.gpr[11]);
  eprintf("$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n", cpu.gpr[12], cpu.gpr[13], cpu.gpr[14], cpu.gpr[15]);
  eprintf("$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n", cpu.gpr[16], cpu.gpr[17], cpu.gpr[18], cpu.gpr[19]);
  eprintf("$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n", cpu.gpr[20], cpu.gpr[21], cpu.gpr[22], cpu.gpr[23]);
  eprintf("$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n", cpu.gpr[24], cpu.gpr[25], cpu.gpr[26], cpu.gpr[27]);
  eprintf("$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n", cpu.gpr[28], cpu.gpr[29], cpu.gpr[30], cpu.gpr[31]);
  // =============================================================
    eprintf("$count0:%08x,    $count1:%08x\n", cpu.cp0.count[0], cpu.cp0.count[1]);
    // eprintf("$compare:%08x,    $status:%08x,    $cause:%08x\n", cpu.cp0[CP0_COMPARE][0], cpu.cp0[CP0_STATUS][0], cpu.cp0[CP0_CAUSE][0]);
    eprintf("$epc:%08x\n", cpu.cp0.epc);
  // =============================================================
}


int init_cpu(vaddr_t entry) {
  nemu_start_time = get_current_time();

  cpu.pc = entry;
  cpu.cp0.cpr[CP0_STATUS][0] = 0x1000FF00;
  cpu.cp0.cpr[CP0_PRID][0] = 0x00018000;   // MIPS32 4Kc

  // init cp0 config 0
  cpu.cp0.config.MT = 1; // standard MMU
  cpu.cp0.config.BE = 0; // little endian
  cpu.cp0.config.M  = 1; // config1 present

  // init cp0 config 1
  cpu.cp0.config1.DA = 2; // 4=2^(2) ways dcache
  cpu.cp0.config1.DL = 2; // 8=2^(2 + 1) bytes per line
  cpu.cp0.config1.DS = 0; // 64=2^(0 + 8) sets

  cpu.cp0.config1.IA = 2; // 4=2^(2) ways dcache
  cpu.cp0.config1.IL = 2; // 8=2^(2 + 1) bytes per line
  cpu.cp0.config1.IS = 0; // 64=2^(0 + 8) sets

  cpu.cp0.config1.MMU_size = 63; // 64 TLB entries

  return 0;
}

static inline uint32_t instr_fetch(uint32_t addr) {
  if(is_unmapped(addr)) {
	extern uint32_t unmapped[];
	addr -= UNMAPPED_BASE;
	Assert(addr < UNMAPPED_SIZE && (addr & 3) == 0,
		"addr is %08x, UNMAPPED_BASE:%08x\n", addr + UNMAPPED_BASE, UNMAPPED_BASE);
	return unmapped[addr >> 2];
  } else {
	addr = prot_addr(addr) - DDR_BASE;
	Assert(addr < DDR_SIZE && (addr & 3) == 0,
		"addr is %08x, DDR_BASE:%08x\n", addr + DDR_BASE, DDR_BASE);
	return ((uint32_t*)ddr)[addr >> 2];
  }
}

static inline uint32_t load_mem(vaddr_t addr, int len) {
  uint32_t pa = prot_addr(addr);
  if(LIKELY(DDR_BASE <= pa && pa < DDR_BASE + DDR_SIZE)) {
	Assert(0, "addr:%08x, pa:%08x\n", addr, pa);
    addr = pa - DDR_BASE;
	switch(len) {
	  case 1: return ddr[addr];
	  case 2: return (ddr[addr + 1] << 8) | ddr[addr];
	  case 3: return (ddr[addr + 2] << 16) | (ddr[addr + 1] << 8) | ddr[addr];
	  case 4: return (ddr[addr + 3] << 24) | (ddr[addr + 2] << 16) | (ddr[addr + 1] << 8) | ddr[addr];
	  default: CPUAssert(0, "invalid len %d\n", len); break;
	}
  }
  return vaddr_read(addr, len);
}

static inline void store_mem(vaddr_t addr, int len, uint32_t data) {
  uint32_t pa = prot_addr(addr);
  if(LIKELY(DDR_BASE <= pa && pa < DDR_BASE + DDR_SIZE)) {
	Assert(0, "addr:%08x, pa:%08x\n", addr, pa);
    addr = pa - DDR_BASE;
	switch(len) {
	  case 1: ddr[addr] = data; return;
	  case 2: ddr[addr] = data & 0xFF;
			  ddr[addr + 1] = (data >> 8) & 0xFF;
			  return;
	  case 3: ddr[addr] = data & 0xFF;
			  ddr[addr + 1] = (data >> 8) & 0xFF;
			  ddr[addr + 2] = (data >> 16) & 0xFF;
			  return;
	  case 4: ddr[addr] = data & 0xFF;
			  ddr[addr + 1] = (data >> 8) & 0xFF;
			  ddr[addr + 2] = (data >> 16) & 0xFF;
			  ddr[addr + 3] = (data >> 24) & 0xFF;
			  return;
	  default: CPUAssert(0, "invalid len %d\n", len); break;
	}
  } else {
    vaddr_write(addr, len, data);
  }
}

void signal_exception(int code) {
  if(code == EXC_TRAP) {
	eprintf("\e[31mHIT BAD TRAP txx\e0m\n");
	exit(0);
  }

  cpu.cp0.epc = cpu.pc;
  cpu.pc = EXCEPTION_VECTOR_LOCATION;

#ifdef ENABLE_SEGMENT
  cpu.base = 0; // kernel segment base is zero
#endif

  cpu.cp0.status.EXL = 1;
  cpu.cp0.status.IE = 0;

  cpu.cp0.cause.ExcCode = code;

#ifdef ENABLE_PAGING
  longjmp(cpu_exception_handler, 1);
#endif
}


void check_interrupt(bool ie) {
  if(ie && cpu.cp0.cause.IP) {
	signal_exception(EXC_INTR);
  }
}

void update_cp0_timer() {
  union { struct { uint32_t lo, hi; }; uint64_t val; } cycles;
  cycles.lo = cpu.cp0.count[0];
  cycles.hi = cpu.cp0.count[1];
  cycles.val += 5; // add 5 cycles
  cpu.cp0.count[0] = cycles.lo;
  cpu.cp0.count[1] = cycles.hi;

  // update IP
  if(cpu.cp0.count[0] == cpu.cp0.compare) {
    cpu.cp0.cause.IP |= CAUSE_IP_TIMER;
  }
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if(work_mode == MODE_GDB && nemu_state != NEMU_END) {
	/* assertion failure handler */
	extern jmp_buf gdb_mode_top_caller;
	int code = setjmp(gdb_mode_top_caller);
	if(code != 0) nemu_state = NEMU_END;
  }

  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }

  nemu_state = NEMU_RUNNING;

#ifdef ENABLE_PAGING
  /* register exception handler */
  setjmp(cpu_exception_handler);
#endif

  for (; n > 0; n --) {
#ifdef ENABLE_INTR
	update_cp0_timer();
#endif
	
	oldpc = cpu.pc;

#if 0
    asm_buf_p = asm_buf;
    asm_buf_p += dsprintf(asm_buf_p, "%8x:    ", cpu.pc);
#endif

    Inst inst = { .val = instr_fetch(cpu.pc) };

	cpu.pc += 4;

#ifdef ENABLE_INTR
	bool ie = !(cpu.cp0.status.EXL) && cpu.cp0.status.IE;
#endif

    asm_buf_p += dsprintf(asm_buf_p, "%08x    ", inst.val);

#include "exec-handlers.h"

    if(work_mode == MODE_LOG) print_registers();

#ifdef ENABLE_INTR
    check_interrupt(ie);
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
