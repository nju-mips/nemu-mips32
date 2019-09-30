#include <setjmp.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "debug.h"
#include "device.h"
#include "memory.h"
#include "mmu.h"
#include "monitor.h"
#include "nemu.h"

/* some global bariables */
nemu_state_t nemu_state = NEMU_STOP;
CPU_state cpu;

static uint64_t nemu_start_time = 0;

/* clang-format off */
const char *regs[32] = {
  "0 ", "at", "v0", "v1",
  "a0", "a1", "a2", "a3",
  "t0", "t1", "t2", "t3",
  "t4", "t5", "t6", "t7",
  "s0", "s1", "s2", "s3",
  "s4", "s5", "s6", "s7",
  "t8", "t9", "k0", "k1",
  "gp", "sp", "fp", "ra"
};
/* clang-format on */

/* APIs from iq.c */
extern uint32_t get_current_pc();
extern uint32_t get_current_instr();

#define UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#define LIKELY(cond) __builtin_expect(!!(cond), 1)
#define ALWAYS_INLINE static inline __attribute__((always_inline))

#define MAX_INSTR_TO_PRINT 10

// 1s = 10^3 ms = 10^6 us
uint64_t get_current_time() { // in us
#if 1
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec * 1000000 + t.tv_usec - nemu_start_time;
#elif 0
  clockid_t clockid = {0};
  struct timespec tp = {0};
  clock_getcpuclockid(getpid(), &clockid);
  clock_gettime(clockid, &tp);
  // clock_gettime(CLOCK_THREAD_CPUTIME_ID, &tp);
  return (tp.tv_sec * 1000000 + tp.tv_nsec / 1000) - nemu_start_time;
#elif 0
  struct rusage usage = {0};
  getrusage(RUSAGE_SELF /* current thread */, &usage);
  return (usage.ru_utime.tv_sec * 1000000 + usage.ru_utime.tv_usec) -
         nemu_start_time;
#endif
}

void print_registers(void) {
  static unsigned int ninstr = 0;
  eprintf("$pc:    0x%08x", get_current_pc());
  eprintf("   ");
  eprintf("$hi:    0x%08x", cpu.hi);
  eprintf("   ");
  eprintf("$lo:    0x%08x", cpu.lo);
  eprintf("\n");

  eprintf("$ninstr: %08x", ninstr);
  eprintf("                  ");
  eprintf("$instr: %08x", get_current_instr());
  eprintf("\n");

  for (int i = 0; i < 32; i++) {
    eprintf("$%s:0x%08x%c", regs[i], cpu.gpr[i], (i + 1) % 4 == 0 ? '\n' : ' ');
  }

  ninstr++;
}

/* if you run your os with multiple processes, disable this
 */
#ifdef ENABLE_CAE_CHECK

#  define NR_GPR 32
static uint32_t saved_gprs[NR_GPR];

void save_usual_registers(void) {
  for (int i = 0; i < NR_GPR; i++) saved_gprs[i] = cpu.gpr[i];
}

void check_usual_registers(void) {
  for (int i = 0; i < NR_GPR; i++) {
    if (i == R_k0 || i == R_k1) continue;
    CPUAssert(saved_gprs[i] == cpu.gpr[i], "gpr[%d] %08x <> %08x after eret\n",
        i, saved_gprs[i], cpu.gpr[i]);
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

#ifdef PERF_SOFTMMU
uint64_t softmmu_hit = 0;
uint64_t softmmu_miss = 0;
#endif

#define MMU_BITS 12

struct softmmu_t {
  uint32_t id;
  uint8_t *ptr;
};

static struct softmmu_t softmmu[1 << MMU_BITS];

static inline void clear_softmmu() {
  for (int i = 0; i < sizeof(softmmu) / sizeof(*softmmu); i++) {
    softmmu[i].id = 0xFFFFFFFF;
    softmmu[i].ptr = NULL;
  }
}

ALWAYS_INLINE uint32_t softmmu_index(vaddr_t vaddr) {
  return (vaddr >> 12) & ((1 << MMU_BITS) - 1);
}

ALWAYS_INLINE uint32_t softmmu_id(vaddr_t vaddr) {
  return (vaddr >> (12 + MMU_BITS));
}

static void update_softmmu(vaddr_t vaddr, paddr_t paddr, device_t *dev) {
#ifdef PERF_SOFTMMU
  softmmu_miss++;
#endif
  if (dev->map) {
    uint32_t idx = softmmu_index(vaddr);
    softmmu[idx].id = softmmu_id(vaddr);
    softmmu[idx].ptr = dev->map((paddr & ~0xFFF) - dev->start, 0);
  }
}

ALWAYS_INLINE uint32_t load_mem(vaddr_t addr, int len) {
  uint32_t idx = softmmu_index(addr);
  if (softmmu[idx].id == softmmu_id(addr)) {
#ifdef PERF_SOFTMMU
    softmmu_hit++;
#endif
    return *((uint32_t *)&softmmu[idx].ptr[addr & 0xFFF]) &
           (~0u >> ((4 - len) << 3));
  } else {
    paddr_t paddr = prot_addr(addr, MMU_LOAD);
    device_t *dev = find_device(paddr);
    CPUAssert(dev && dev->read, "bad addr %08x\n", addr);
    update_softmmu(addr, paddr, dev);
    return dev->read(paddr - dev->start, len);
  }
}

ALWAYS_INLINE void store_mem(vaddr_t addr, int len, uint32_t data) {
  uint32_t idx = softmmu_index(addr);
  if (softmmu[idx].id == softmmu_id(addr)) {
#ifdef PERF_SOFTMMU
    softmmu_hit++;
#endif
    memcpy(&softmmu[idx].ptr[addr & 0xFFF], &data, len);
  } else {
    paddr_t paddr = prot_addr(addr, MMU_STORE);
    device_t *dev = find_device(paddr);
    CPUAssert(dev && dev->write, "bad addr %08x\n", addr);
    update_softmmu(addr, paddr, dev);
    dev->write(paddr - dev->start, len, data);
  }
}

#ifdef PERF_PREDECODE
uint64_t predecode_hit = 0;
uint64_t predecode_miss = 0;
#endif

#define PREDECODE_BITS 12
typedef struct {
  const void *handler;
  uint32_t id;
  union {
    struct {
      int rs, rt; // R and I

      union {
        struct {
          int rd;
          int shamt;
          int func;
        }; // R

        union {
          uint16_t uimm;
          int16_t simm;
        }; // I
      };   // R and I union
    };     // R and I union

    uint32_t addr; // J
  };

  int sel; /* put here will improve performance */

#ifdef DEBUG
  Inst inst;
#endif
} predecode_t;

static predecode_t predecode[1 << PREDECODE_BITS];

static inline void clear_predecode() {
  for (int i = 0; i < sizeof(predecode) / sizeof(*predecode); i++) {
    predecode[i].handler = 0;
  }
}

ALWAYS_INLINE uint32_t predecode_index(vaddr_t vaddr) {
  return vaddr & ((1 << PREDECODE_BITS) - 1);
}

ALWAYS_INLINE uint32_t predecode_id(vaddr_t vaddr) {
  return (vaddr >> PREDECODE_BITS);
}

ALWAYS_INLINE predecode_t *instr_fetch(vaddr_t pc) {
  uint32_t idx = predecode_index(pc);
  uint32_t id = predecode_id(pc);
  if (predecode[idx].id != id) {
    predecode[idx].handler = NULL;
    predecode[idx].id = id;
  }
  return &predecode[idx];
}

void signal_exception(int code) {
  if (code == EXC_TRAP) { panic("HIT BAD TRAP @%08x\n", get_current_pc()); }

#ifdef ENABLE_CAE_CHECK
  save_usual_registers();
#endif

  if (cpu.is_delayslot) {
    cpu.cp0.epc = cpu.pc - 4;
    cpu.cp0.cause.BD = cpu.is_delayslot && cpu.cp0.status.EXL == 0;
    cpu.is_delayslot = false;
  } else {
    cpu.cp0.epc = cpu.pc;
  }

  // eprintf("signal exception %d@%08x, badvaddr:%08x\n",
  // code, cpu.pc, cpu.cp0.badvaddr);

  cpu.has_exception = true;

#ifdef __ARCH_LOONGSON__
  /* for loongson testcase, the only exception entry is
   * 'h0380' */
  cpu.br_target = 0xbfc00380;
  if (code == EXC_TLBM || code == EXC_TLBL || code == EXC_TLBS)
    cpu.br_target = 0xbfc00000;
#else
  /* reference linux: arch/mips/kernel/cps-vec.S */
  switch (code) {
  case EXC_INTR:
    if (cpu.cp0.status.BEV) {
      if (cpu.cp0.cause.IV) {
        cpu.br_target = 0xbfc00400;
      } else {
        cpu.br_target = 0xbfc00380;
      }
    } else {
      if (cpu.cp0.cause.IV) {
        cpu.br_target = 0x80000200;
      } else {
        cpu.br_target = 0x80000180;
      }
    }
    break;
  case EXC_TLBM:
  case EXC_TLBL:
  case EXC_TLBS:
    if (cpu.cp0.status.BEV) {
      if (cpu.cp0.status.EXL) {
        cpu.br_target = 0xbfc00380;
      } else {
        cpu.br_target = 0xbfc00200;
      }
    } else {
      if (cpu.cp0.status.EXL) {
        cpu.br_target = 0x80000180;
      } else {
        cpu.br_target = 0x80000000;
      }
    }
  default:
    if (cpu.cp0.status.BEV) {
      cpu.br_target = 0xbfc00380;
    } else {
      cpu.br_target = 0x80000180;
    }
  }
#endif

#ifdef ENABLE_SEGMENT
  cpu.base = 0; // kernel segment base is zero
#endif

  cpu.cp0.status.EXL = 1;

  cpu.cp0.cause.ExcCode = code;
}

#if defined(ENABLE_EXCEPTION) || defined(ENABLE_INTR)
static inline void check_intrs() {
  bool ie = !(cpu.cp0.status.ERL) && !(cpu.cp0.status.EXL) && cpu.cp0.status.IE;
  if (ie && (cpu.cp0.status.IM & cpu.cp0.cause.IP)) {
    signal_exception(EXC_INTR);

    /* next cycle the IP will be cleared */
    if (cpu.cp0.cause.IP & CAUSE_IP_TIMER) cpu.cp0.cause.IP &= ~CAUSE_IP_TIMER;
  }
}
#endif

void update_cp0_timer() {
  ++(*(uint64_t *)cpu.cp0.count);

  // update IP
  if (cpu.cp0.compare != 0 && cpu.cp0.count[0] == cpu.cp0.compare) {
    cpu.cp0.cause.IP |= CAUSE_IP_TIMER;
  }
}

void nemu_exit() {
#ifdef PERF_SOFTMMU
  printf("softmmu: %lu/%lu = %lf\n", softmmu_hit, softmmu_hit + softmmu_miss,
      softmmu_hit / (double)(softmmu_hit + softmmu_miss));
#endif

#ifdef PERF_PREDECODE
  printf("predecode: %lu/%lu = %lf\n", predecode_hit,
      predecode_hit + predecode_miss,
      predecode_hit / (double)(predecode_hit + predecode_miss));
#endif

  exit(0);
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
        "Program execution has ended. To restart the program, exit NEMU and "
        "run again.\n");
    return;
  }

  nemu_state = NEMU_RUNNING;

  predecode_t *decode = NULL;
#include "exec-handlers.h"

  return;
}
