#include <elf.h>
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
#include "utils.h"

#define ALWAYS_INLINE inline __attribute__((always_inline))

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

#define UNLIKELY(cond) __builtin_expect(!!(cond), 0)
#define LIKELY(cond) __builtin_expect(!!(cond), 1)

#define MAX_INSTR_TO_PRINT 10


void nemu_set_irq(int irqno, bool val) {
  assert(0 <= irqno && irqno < 8);
  if (val) {
    cpu.cp0.cause.IP |= 1 << irqno;
  } else {
    cpu.cp0.cause.IP &= ~(1 << irqno);
  }
}

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
#if CONFIG_INSTR_LOG
  eprintf("$pc:    0x%08x", get_current_pc());
#else
  eprintf("$pc:    ????????");
#endif
  eprintf("   ");
  eprintf("$hi:    0x%08x", cpu.hi);
  eprintf("   ");
  eprintf("$lo:    0x%08x", cpu.lo);
  eprintf("\n");

  eprintf("$ninstr: %08x", ninstr);
  eprintf("                  ");
#if CONFIG_INSTR_LOG
  eprintf("$instr: %08x", get_current_instr());
#else
  eprintf("$instr: ????????");
#endif
  eprintf("\n");

  for (int i = 0; i < 32; i++) {
    eprintf("$%s:0x%08x%c", regs[i], cpu.gpr[i], (i + 1) % 4 == 0 ? '\n' : ' ');
  }

  ninstr++;
}

/* if you run your os with multiple processes, disable this
 */
#if CONFIG_CAE_CHECK

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

#if CONFIG_MMU_CACHE_PERF
uint64_t mmu_cache_hit = 0;
uint64_t mmu_cache_miss = 0;
#endif

#define MMU_BITS 12

struct mmu_cache_t {
  uint32_t id;
  uint8_t *ptr;
  bool can_write;
};

static struct mmu_cache_t mmu_cache[1 << MMU_BITS];

static inline void clear_mmu_cache() {
  for (int i = 0; i < sizeof(mmu_cache) / sizeof(*mmu_cache); i++) {
    mmu_cache[i].id = 0xFFFFFFFF;
    mmu_cache[i].ptr = NULL;
  }
}

static ALWAYS_INLINE uint32_t mmu_cache_index(vaddr_t vaddr) {
  return (vaddr >> 12) & ((1 << MMU_BITS) - 1);
}

static ALWAYS_INLINE uint32_t mmu_cache_id(vaddr_t vaddr) {
  return (vaddr >> (12 + MMU_BITS));
}

static ALWAYS_INLINE void update_mmu_cache(
    vaddr_t vaddr, paddr_t paddr, device_t *dev, bool can_write) {
#if CONFIG_MMU_CACHE_PERF
  mmu_cache_miss++;
#endif
  if (cpu.has_exception) return;
  if (dev->map) {
    uint32_t idx = mmu_cache_index(vaddr);
    mmu_cache[idx].id = mmu_cache_id(vaddr);
    mmu_cache[idx].ptr = dev->map((paddr & ~0xFFF) - dev->start, 0);
    mmu_cache[idx].can_write = can_write;
    assert(mmu_cache[idx].ptr);
  }
}

static ALWAYS_INLINE uint32_t vaddr_read(vaddr_t addr, int len) {
  uint32_t idx = mmu_cache_index(addr);
  if (CONFIG_IS_ENABLED(MMU_CACHE) && mmu_cache[idx].id == mmu_cache_id(addr)) {
#if CONFIG_MMU_CACHE_PERF
    mmu_cache_hit++;
#endif
    uint32_t data = *((uint32_t *)&mmu_cache[idx].ptr[addr & 0xFFF]) &
                    (~0u >> ((4 - len) << 3));
#if CONFIG_MMU_CACHE_CHECK
    assert(data == dbg_vaddr_read(addr, len));
#endif
    return data;
  } else {
    mmu_attr_t attr = {.rwbit = MMU_LOAD, .exbit = 1};
    paddr_t paddr = prot_addr_with_attr(addr, &attr);
    device_t *dev = find_device(paddr);
    CPUAssert(dev && dev->read, "bad addr %08x\n", addr);
    update_mmu_cache(addr, paddr, dev, attr.dirty);
    uint32_t data = dev->read(paddr - dev->start, len);
#if CONFIG_MMIO_ACCESS_LOG
    if (strcmp(CONFIG_MMIO_ACCESS_LOG_DEVICE, dev->name) == 0) {
      eprintf("[NEMU] R(%s, %08x, %d) -> %08x\n", dev->name, paddr - dev->start,
          len, data);
    }
#endif
    return data;
  }
}

static ALWAYS_INLINE void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  uint32_t idx = mmu_cache_index(addr);
  if (CONFIG_IS_ENABLED(MMU_CACHE) && mmu_cache[idx].id == mmu_cache_id(addr) &&
      mmu_cache[idx].can_write) {
#if CONFIG_MMU_CACHE_PERF
    mmu_cache_hit++;
#endif
#if CONFIG_MMU_CACHE_CHECK
    extern device_t blackhole_dev;
    paddr_t paddr = prot_addr(addr, MMU_STORE);
    assert(paddr != blackhole_dev.start);
    device_t *dev = find_device(paddr);
    assert(dev && dev->map);
    assert(mmu_cache[idx].ptr == dev->map((paddr & ~0xFFF) - dev->start, 0));
#endif
    memcpy(&mmu_cache[idx].ptr[addr & 0xFFF], &data, len);
  } else {
    paddr_t paddr = prot_addr(addr, MMU_STORE);
    device_t *dev = find_device(paddr);
    CPUAssert(dev && dev->write, "bad addr %08x\n", addr);
    update_mmu_cache(addr, paddr, dev, true);
#if CONFIG_MMIO_ACCESS_LOG
    if (strcmp(CONFIG_MMIO_ACCESS_LOG_DEVICE, dev->name) == 0) {
      eprintf("[NEMU] W(%s, %08x, %d) -> %08x\n", dev->name, paddr - dev->start,
          len, data);
    }
#endif
    dev->write(paddr - dev->start, len, data);
  }
}

#if CONFIG_DECODE_CACHE_PERF
uint64_t decode_cache_hit = 0;
uint64_t decode_cache_miss = 0;
#endif

#define DECODE_CACHE_BITS 12
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

#if CONFIG_INSTR_LOG || CONFIG_DECODE_CACHE_CHECK
  Inst inst;
#endif
} decode_cache_t;

static decode_cache_t decode_cache[1 << DECODE_CACHE_BITS];

void clear_decode_cache() {
  for (int i = 0; i < sizeof(decode_cache) / sizeof(*decode_cache); i++) {
    decode_cache[i].handler = NULL;
  }
}

static ALWAYS_INLINE uint32_t decode_cache_index(vaddr_t vaddr) {
  return vaddr & ((1 << DECODE_CACHE_BITS) - 1);
}

static ALWAYS_INLINE uint32_t decode_cache_id(vaddr_t vaddr) {
  return (vaddr >> DECODE_CACHE_BITS);
}

static ALWAYS_INLINE decode_cache_t *decode_cache_fetch(vaddr_t pc) {
  uint32_t idx = decode_cache_index(pc);
  uint32_t id = decode_cache_id(pc);
  if (decode_cache[idx].id != id) {
    decode_cache[idx].handler = NULL;
    decode_cache[idx].id = id;
  }
  return &decode_cache[idx];
}

void launch_exception(uint32_t exception) {
  int code = exception & 0xFFFF;
  int extra = exception >> 16;

  if (code == EXC_TRAP) { panic("HIT BAD TRAP @%08x\n", get_current_pc()); }

#if CONFIG_CAE_CHECK
  save_usual_registers();
#endif

  cpu.has_exception = true;

  uint32_t vecOff = 0;
  if (CONFIG_IS_ENABLED(MARCH_NOOP) || cpu.cp0.status.EXL == 0) {
#if CONFIG_DELAYSLOT
    if (cpu.is_delayslot) {
      cpu.is_delayslot = false; // !!
      cpu.cp0.epc = cpu.pc - 4;
      cpu.cp0.cause.BD = 1;
    } else
#endif
    {
      cpu.cp0.epc = cpu.pc;
      cpu.cp0.cause.BD = 0;
    }

    if (extra == EX_TLB_REFILL)
      vecOff = 0x000;
    else if (code == EXC_INTR && cpu.cp0.cause.IV == 1)
      vecOff = 0x200;
    else
      vecOff = 0x180;
  } else {
    vecOff = 0x180;
  }

  cpu.cp0.cause.ExcCode = code;
  cpu.cp0.status.EXL = 1;
  if (cpu.cp0.status.BEV == 1) {
    cpu.br_target = 0xbfc00200 + vecOff;
  } else {
    cpu.br_target = 0x80000000 + vecOff;
  }

#if CONFIG_SEGMENT
  cpu.base = 0; // kernel segment base is zero
#endif

  clear_mmu_cache();
  clear_decode_cache();
}

int init_cpu(vaddr_t entry) {
  nemu_start_time = get_current_time();

  cpu.cp0.count[0] = 0;
  cpu.cp0.compare = 0xFFFFFFFF;

  cpu.cp0.status.CU = CU0_ENABLE;
  cpu.cp0.status.ERL = 0;
  cpu.cp0.status.BEV = 1;
  cpu.cp0.status.IM = 0x00;

  cpu.cp0.cause.IV = 0;

  cpu.pc = entry;
  cpu.cp0.cpr[CP0_PRID][0] = 0x00018000; // MIPS32 4Kc

  // init cp0 config 0
  cpu.cp0.config.MT = 1; // standard MMU
  cpu.cp0.config.BE = 0; // little endian
  cpu.cp0.config.M = 1;  // config1 present

  // init cp0 config 1
  cpu.cp0.config1.DA = 3; // 4=$3+1 ways dcache
  cpu.cp0.config1.DL = 3; // 16=2^($3 + 1) bytes per line
  cpu.cp0.config1.DS = 2; // 256=2^($2 + 6) sets

  cpu.cp0.config1.IA = 3; // 4=$3+1 ways ways dcache
  cpu.cp0.config1.IL = 3; // 16=2^($3 + 1) bytes per line
  cpu.cp0.config1.IS = 2; // 256=2^($2 + 6) sets

  cpu.cp0.config1.MMU_size = 63; // 64 TLB entries

  /* initialize some cache */
  clear_mmu_cache();
  clear_decode_cache();

  return 0;
}

#if CONFIG_EXCEPTION || CONFIG_INTR
static ALWAYS_INLINE void check_intrs() {
  bool ie = !(cpu.cp0.status.ERL) && !(cpu.cp0.status.EXL) && cpu.cp0.status.IE;
  if (ie && (cpu.cp0.status.IM & cpu.cp0.cause.IP)) {
    launch_exception(EXC_INTR);
  }
}
#endif

static uint64_t intr_ddl = 0;

uint64_t mips_get_count() {
  return get_current_time() * 50; // for 50 MHZ
}

void update_interrupt_deadline() {
  uint64_t count = mips_get_count();
  uint64_t count0 = count & ((1ull << 32) - 1);
  uint64_t compare = cpu.cp0.compare;
  if (compare < count0) compare = compare + (1ull << 32);
  uint64_t intr_interval = compare - count;
  intr_ddl = count + intr_interval;
}

#if CONFIG_INTR
bool check_cp0_timer() {
  /* update IP */
  bool has_intr = false;
  if (mips_get_count() > intr_ddl) {
    nemu_set_irq(7, 1);
    intr_ddl = -1ull;
    has_intr = true;
  }
  return has_intr;
}
#endif

void nemu_epilogue() {
#if CONFIG_MMU_CACHE_PERF
  printf("mmu_cache: %lu/%lu = %lf\n", mmu_cache_hit,
      mmu_cache_hit + mmu_cache_miss,
      mmu_cache_hit / (double)(mmu_cache_hit + mmu_cache_miss));
#endif

#if CONFIG_DECODE_CACHE_PERF
  printf("decode_cache: %lu/%lu = %lf\n", decode_cache_hit,
      decode_cache_hit + decode_cache_miss,
      decode_cache_hit / (double)(decode_cache_hit + decode_cache_miss));
#endif

#if CONFIG_INSTR_LOG
  eprintf(">>>>>> last executed instructions\n");
  print_instr_queue();
  eprintf("\n");
#endif

#if CONFIG_FUNCTION_TRACE_LOG
  eprintf(">>>>>> function invocations\n");
  print_frames();
  eprintf("\n");
#endif

#if CONFIG_BACKTRACE_LOG
  eprintf(">>>>>> functions in stack\n");
  print_backtrace();
  eprintf("\n");
#endif

#if CONFIG_INSTR_LOG
  eprintf(">>>>>> current registers\n");
  print_registers();
  eprintf("\n");
#endif
}

void nemu_exit() {
  nemu_epilogue();
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

  // stop_cpu_when_ulite_send("Starting ");

  if (nemu_state == NEMU_END) {
    printf(
        "Program execution has ended. To restart the "
        "program, exit NEMU and run again.\n");
    return;
  }

#if CONFIG_INSTR_LOG
  bool nemu_needs_commit = work_mode == MODE_LOG;
#endif

  nemu_state = NEMU_RUNNING;

  for (; n > 0; n--) {
#if CONFIG_INSTR_LOG
    instr_enqueue_pc(cpu.pc);
#endif

#if CONFIG_EXCEPTION
    if ((cpu.pc & 0x3) != 0) {
      cpu.cp0.badvaddr = cpu.pc;
      launch_exception(EXC_AdEL);
      goto check_exception;
    }
#endif

    /* should be bad state */
#if CONFIG_WARN_PC_EQUALS_ZERO
    if (cpu.pc == 0x0) {
      printf("[NEMU] warning: cpu.pc == 0\n");
      print_instr_queue();
    }
#endif

#if CONFIG_DECODE_CACHE
#  define operands decode
    decode_cache_t *decode = decode_cache_fetch(cpu.pc);
#else
#  define operands (&inst)
    Inst inst = {.val = vaddr_read(cpu.pc, 4)};
#endif

#include "instr.h"

#if CONFIG_INSTR_LOG
    if (nemu_needs_commit) print_registers();
#endif

#if CONFIG_EXCEPTION || CONFIG_INTR
  check_exception:;
    if (!cpu.has_exception) check_intrs(); /* soft intr */

    if (cpu.has_exception) {
      cpu.has_exception = false;
      cpu.pc = cpu.br_target;
    }
#endif

    if (nemu_state != NEMU_RUNNING) { return; }
  }

  if (nemu_state == NEMU_RUNNING) { nemu_state = NEMU_STOP; }
}
