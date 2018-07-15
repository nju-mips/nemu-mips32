#include <sys/time.h>

#include "nemu.h"
#include "device.h"
#include "monitor/monitor.h"


#define LIKELY(cond) __builtin_expect(!!(cond), 1)


#define EXCEPTION_VECTOR_LOCATION 0x10000020
#define MAX_INSTR_TO_PRINT 10

int nemu_state = NEMU_STOP;
extern int print_commit_log;

static uint64_t nemu_start_time = 0;

char asm_buf[80];
char *asm_buf_p;

// 1s = 10^3 ms = 10^6 us
static uint64_t get_current_time() { // in us
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec * 1000000 + t.tv_usec - nemu_start_time;
}

static int dsprintf(char *buf, const char *fmt, ...) {
  if(!print_commit_log) return 0;
#if 0
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
#endif
  return 0;
}


void print_registers() {
  // printf("$base:     0x%08x\n", cpu.base);
  printf("$pc:    0x%08x    $hi:    0x%08x    $lo:    0x%08x\n", cpu.pc, cpu.hi, cpu.lo);
  printf("$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n", cpu.gpr[0], cpu.gpr[1], cpu.gpr[2], cpu.gpr[3]);
  printf("$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n", cpu.gpr[4], cpu.gpr[5], cpu.gpr[6], cpu.gpr[7]);
  printf("$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n", cpu.gpr[8], cpu.gpr[9], cpu.gpr[10], cpu.gpr[11]);
  printf("$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n", cpu.gpr[12], cpu.gpr[13], cpu.gpr[14], cpu.gpr[15]);
  printf("$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n", cpu.gpr[16], cpu.gpr[17], cpu.gpr[18], cpu.gpr[19]);
  printf("$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n", cpu.gpr[20], cpu.gpr[21], cpu.gpr[22], cpu.gpr[23]);
  printf("$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n", cpu.gpr[24], cpu.gpr[25], cpu.gpr[26], cpu.gpr[27]);
  printf("$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n", cpu.gpr[28], cpu.gpr[29], cpu.gpr[30], cpu.gpr[31]);
}


int init_cpu() {
  nemu_start_time = get_current_time();
  assert(sizeof(cp0_status_t) == sizeof(cpu.cp0[CP0_STATUS][0]));
  assert(sizeof(cp0_cause_t) == sizeof(cpu.cp0[CP0_CAUSE][0]));
  cpu.gpr[29] = 0x18000000;
  cpu.cp0[CP0_STATUS][0] = 0x1000FF00;
  return 0;
}

static inline uint32_t instr_fetch(uint32_t addr) {
  addr = addr - DDR_BASE + cpu.base;
  assert(addr < DDR_SIZE && (addr & 3) == 0);
  cpu.pc += 4;
  return ((uint32_t*)ddr)[addr >> 2];
}

static inline uint32_t load_mem(vaddr_t addr, int len) {
  if(LIKELY(DDR_BASE <= addr && addr < DDR_BASE + DDR_SIZE)) {
    addr = addr - DDR_BASE + cpu.base;
	switch(len) {
	  case 1: return ddr[addr];
	  case 2: return (ddr[addr + 1] << 8) | ddr[addr];
	  case 3: return (ddr[addr + 2] << 16) | (ddr[addr + 1] << 8) | ddr[addr];
	  case 4: return (ddr[addr + 3] << 24) | (ddr[addr + 2] << 16) | (ddr[addr + 1] << 8) | ddr[addr];
	}
  }
  return vaddr_read(addr, len);
}

static inline void store_mem(vaddr_t addr, int len, uint32_t data) {
  if(LIKELY(DDR_BASE <= addr && addr < DDR_BASE + DDR_SIZE)) {
    addr = addr - DDR_BASE + cpu.base;
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
	}
  } else {
    vaddr_write(addr, len, data);
  }
}

static inline void trigger_exception(int code) {
  cpu.cp0[CP0_EPC][0] = cpu.pc;
  cpu.pc = EXCEPTION_VECTOR_LOCATION;

  cpu.base = 0; // kernel base is zero

  cp0_status_t *status = (void *)&(cpu.cp0[CP0_STATUS][0]);
  status->EXL = 1;
  status->IE = 0;

  cp0_cause_t *cause = (void *)&(cpu.cp0[CP0_CAUSE][0]);
  cause->ExcCode = code;
}


void check_interrupt() {
  if(cpu.cp0[CP0_COMPARE][0] == cpu.cp0[CP0_COUNT][0]) {
	trigger_exception(EXC_INTR);
  }
}

void update_cp0_timer() {
  union { struct { uint32_t lo, hi; }; uint64_t val; } cycles;
  cycles.lo = cpu.cp0[CP0_COUNT][0];
  cycles.hi = cpu.cp0[CP0_COUNT][1];
  cycles.val += 5; // add 5 cycles
  cpu.cp0[CP0_COUNT][0] = cycles.lo;
  cpu.cp0[CP0_COUNT][1] = cycles.hi;
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (nemu_state == NEMU_END) {
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  }
  nemu_state = NEMU_RUNNING;

  for (; n > 0; n --) {
	update_cp0_timer();
	
    asm_buf_p = asm_buf;
    asm_buf_p += dsprintf(asm_buf_p, "%8x:    ", cpu.pc);

    Inst inst = { .val = instr_fetch(cpu.pc) };

    asm_buf_p += dsprintf(asm_buf_p, "%08x    ", inst.val);

#include "exec-handlers.h"

    if(print_commit_log) print_registers();

#ifdef INTR
    check_interrupt();
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
