#include <setjmp.h>
#include <sys/time.h>

#include "device.h"
#include "memory.h"
#include "mmu.h"
#include "monitor.h"
#include "nemu.h"

CPU_state cpu;

void signal_exception(int code);

const char *regs[32] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0",   "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8",   "t9", "k0", "k1", "gp", "sp", "fp", "ra"};

#define UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#define LIKELY(cond) __builtin_expect(!!(cond), 1)

#define MAX_INSTR_TO_PRINT 10

nemu_state_t nemu_state = NEMU_STOP;

static uint64_t nemu_start_time = 0;

char asm_buf[80], *asm_buf_p;

// 1s = 10^3 ms = 10^6 us
uint64_t get_current_time() { // in us
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec * 1000000 + t.tv_usec - nemu_start_time;
}

static int dsprintf(char *buf, const char *fmt, ...) {
  int len = 0;
#if 0
  va_list ap;
  va_start(ap, fmt);
  len = vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
#endif
  return len;
}

void print_registers(uint32_t instr) {
  static unsigned int ninstr = 0;
  // print registers to stderr, so that will not mixed with
  // uart output
  eprintf(
      "$pc:    0x%08x    $hi:    0x%08x    $lo:    "
      "0x%08x\n",
      cpu.pc, cpu.hi, cpu.lo);
  eprintf(
      "$ninstr: 0x%08x                     $instr: "
      "0x%08x\n",
      ninstr, instr);
  eprintf(
      "$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n",
      cpu.gpr[0], cpu.gpr[1], cpu.gpr[2], cpu.gpr[3]);
  eprintf(
      "$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n",
      cpu.gpr[4], cpu.gpr[5], cpu.gpr[6], cpu.gpr[7]);
  eprintf(
      "$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n",
      cpu.gpr[8], cpu.gpr[9], cpu.gpr[10], cpu.gpr[11]);
  eprintf(
      "$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n",
      cpu.gpr[12], cpu.gpr[13], cpu.gpr[14], cpu.gpr[15]);
  eprintf(
      "$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n",
      cpu.gpr[16], cpu.gpr[17], cpu.gpr[18], cpu.gpr[19]);
  eprintf(
      "$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n",
      cpu.gpr[20], cpu.gpr[21], cpu.gpr[22], cpu.gpr[23]);
  eprintf(
      "$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n",
      cpu.gpr[24], cpu.gpr[25], cpu.gpr[26], cpu.gpr[27]);
  eprintf(
      "$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n",
      cpu.gpr[28], cpu.gpr[29], cpu.gpr[30], cpu.gpr[31]);
  ninstr++;
}

/* if you run your os with multiple processes, disable this
 */
#ifdef ENABLE_CAE_CHECK

#  define NR_GPR 32
static uint32_t saved_gprs[NR_GPR];

void save_usual_registers(void) {
  for (int i = 0; i < NR_GPR; i++)
    saved_gprs[i] = cpu.gpr[i];
}

void check_usual_registers(void) {
  for (int i = 0; i < NR_GPR; i++) {
    if (i == 26 || i == 27) continue; // k0 and k1
    CPUAssert(saved_gprs[i] == cpu.gpr[i],
              "gpr[%d] %08x <> %08x after eret\n", i,
              saved_gprs[i], cpu.gpr[i]);
  }
}

#endif

int init_cpu(vaddr_t entry) {
  nemu_start_time = get_current_time();

  cpu.cp0.count[0] = 0;
  cpu.cp0.compare = 0xFFFFFFFF;

  cpu.cp0.status.CU = CU0_ENABLE;
  cpu.cp0.status.ERL = 1;
  cpu.cp0.status.BEV = 1;
  cpu.cp0.status.IM = 0x00;

  cpu.pc = entry;
  cpu.cp0.cpr[CP0_PRID][0] = 0x00018000; // MIPS32 4Kc

  // init cp0 config 0
  cpu.cp0.config.MT = 1; // standard MMU
  cpu.cp0.config.BE = 0; // little endian
  cpu.cp0.config.M = 1;  // config1 present

  // init cp0 config 1
  cpu.cp0.config1.DA = 3; // 4=3+1 ways dcache
  cpu.cp0.config1.DL = 1; // 4=2^(1 + 1) bytes per line
  cpu.cp0.config1.DS = 2; // 256=2^(2 + 6) sets

  cpu.cp0.config1.IA = 3; // 4=3+1 ways ways dcache
  cpu.cp0.config1.IL = 1; // 4=2^(1 + 1) bytes per line
  cpu.cp0.config1.IS = 2; // 256=2^(2 + 6) sets

  cpu.cp0.config1.MMU_size = 63; // 64 TLB entries

  return 0;
}

static inline uint32_t read_handler(uint8_t *p, int len) {
  return *((uint32_t *)((void *)p)) &
         (~0u >> ((4 - len) << 3));
}

static inline void write_handler(uint8_t *p, uint32_t len,
                                 uint32_t data) {
  memcpy(p, &data, len);
}

static inline uint32_t load_mem(vaddr_t addr, int len) {
#ifdef DEBUG
  CPUAssert(addr >= 0x1000,
            "preventive check failed, try to load memory "
            "from addr %08x\n",
            addr);
#endif
  uint32_t pa = prot_addr(addr, MMU_LOAD);
  if (LIKELY(DDR_BASE <= pa && pa < DDR_BASE + DDR_SIZE)) {
    // Assert(0, "addr:%08x, pa:%08x\n", addr, pa);
    addr = pa - DDR_BASE;
    return read_handler(&ddr[addr], len);
  } else if (BRAM_BASE <= pa &&
             pa < BRAM_BASE + BRAM_SIZE) {
    addr = pa - BRAM_BASE;
    return read_handler(&bram[addr], len);
  }
  return vaddr_read(addr, len);
}

static inline void store_mem(vaddr_t addr, int len,
                             uint32_t data) {
#ifdef DEBUG
  CPUAssert(addr >= 0x1000,
            "preventive check failed, try to store memory "
            "to addr %08x\n",
            addr);
#endif
  uint32_t pa = prot_addr(addr, MMU_STORE);
  if (LIKELY(DDR_BASE <= pa && pa < DDR_BASE + DDR_SIZE)) {
    addr = pa - DDR_BASE;
    write_handler(&ddr[addr], len, data);
  } else {
    vaddr_write(addr, len, data);
  }
}

static inline uint32_t instr_fetch(vaddr_t addr) {
  return load_mem(addr, 4);
}

void signal_exception(int code) {
  cpu.curr_instr_except = true;

  if (code == EXC_TRAP) {
    eprintf("\e[31mHIT BAD TRAP @%08x\e[0m\n", cpu.pc);
    exit(0);
  }

#ifdef ENABLE_CAE_CHECK
  save_usual_registers();
#endif

  if (cpu.is_delayslot) {
    cpu.cp0.epc = cpu.pc - 4;
    cpu.cp0.cause.BD =
        cpu.is_delayslot && cpu.cp0.status.EXL == 0;
  } else {
    cpu.cp0.epc = cpu.pc;
  }

  // eprintf("signal exception %d@%08x, badvaddr:%08x\n",
  // code, cpu.pc, cpu.cp0.badvaddr);

  /* reference linux: arch/mips/kernel/cps-vec.S */
  // uint32_t ebase = cpu.cp0.status.BEV ? 0xbfc00000 :
  // 0x80000000;
  uint32_t ebase = 0xbfc00000;
  cpu.need_br = true;
  // for loongson testcase, exception entry is 'h0380'
  switch (code) {
  case EXC_INTR:
#ifdef __ARCH_LOONGSON__
    cpu.br_target = ebase + 0x0380;
#else
    if (cpu.cp0.cause.IV) {
      cpu.br_target = ebase + 0x0200;
    } else {
      cpu.br_target = ebase + 0x0180;
    }
#endif
    break;
  case EXC_TLBM:
  case EXC_TLBL:
  case EXC_TLBS: cpu.br_target = ebase + 0x0000; break;
  default: /* usual exception */
#ifdef __ARCH_LOONGSON__
    cpu.br_target = ebase + 0x0380;
#else
    cpu.br_target = ebase + 0x0180;
    break;
#endif
  }

#ifdef ENABLE_SEGMENT
  cpu.base = 0; // kernel segment base is zero
#endif

  cpu.cp0.status.EXL = 1;

  cpu.cp0.cause.ExcCode = code;
}

void check_ipbits(bool ie) {
  if (ie && (cpu.cp0.status.IM & cpu.cp0.cause.IP)) {
    signal_exception(EXC_INTR);
  }
}

void update_cp0_timer() {
  union {
    struct {
      uint32_t lo, hi;
    };
    uint64_t val;
  } cycles;
  cycles.lo = cpu.cp0.count[0];
  cycles.hi = cpu.cp0.count[1];
  cycles.val += 1; // add 5 cycles
  cpu.cp0.count[0] = cycles.lo;
  cpu.cp0.count[1] = cycles.hi;

  // update IP
  if (cpu.cp0.compare != 0 &&
      cpu.cp0.count[0] == cpu.cp0.compare) {
    cpu.cp0.cause.IP |= CAUSE_IP_TIMER;
  }
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  if (work_mode == MODE_GDB && nemu_state != NEMU_END) {
    /* assertion failure handler */
    extern jmp_buf gdb_mode_top_caller;
    int code = setjmp(gdb_mode_top_caller);
    if (code != 0) nemu_state = NEMU_END;
  }

  if (nemu_state == NEMU_END) {
    printf(
        "Program execution has ended. To restart the "
        "program, exit NEMU and run again.\n");
    return;
  }

  nemu_state = NEMU_RUNNING;

  for (; n > 0; n--) {
#ifdef ENABLE_INTR
    update_cp0_timer();
#endif

    if (UNLIKELY(cpu.need_br)) {
      cpu.pc = cpu.br_target;
      cpu.need_br = false;
    }

#ifdef DEBUG
    instr_enqueue_pc(cpu.pc);
#endif

#if 0
    asm_buf_p = asm_buf;
    asm_buf_p += dsprintf(asm_buf_p, "%8x:    ", cpu.pc);
#endif

#ifdef ENABLE_EXCEPTION
    if ((cpu.pc & 0x3) != 0) {
      cpu.cp0.badvaddr = cpu.pc;
      signal_exception(EXC_AdEL);
      goto cpu_exec_end;
    }
#endif

    Inst inst = {.val = instr_fetch(cpu.pc)};

    cpu.curr_instr_except = false;

#ifdef DEBUG
    instr_enqueue_instr(inst.val);
#endif

#if defined(ENABLE_EXCEPTION) || defined(ENABLE_INTR)
    static bool ie = 0; /* fuck gcc */
    ie = !(cpu.cp0.status.ERL) && !(cpu.cp0.status.EXL) &&
         cpu.cp0.status.IE;
#endif

    asm_buf_p += dsprintf(asm_buf_p, "%08x    ", inst.val);

#include "exec-handlers.h"

    if (cpu.is_delayslot) { cpu.need_br = true; }

    cpu.is_delayslot = false; // clear this bits

  cpu_exec_end:

#ifdef DEBUG
    // eprintf("%08x: %08x\n", cpu.pc, inst.val);
    if (work_mode == MODE_LOG) print_registers(inst.val);
#endif

    /* update pc */
    cpu.pc += 4;

#if defined(ENABLE_EXCEPTION) || defined(ENABLE_INTR)
    check_ipbits(ie);
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) {
    nemu_state = NEMU_STOP;
  }
}
