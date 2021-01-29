#include <elf.h>
#include <limits.h>
#include <math.h>
#include <setjmp.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "cpu/decode.h"
#include "cpu/memory.h"
#include "cpu/mmu.h"
#include "debug.h"
#include "device.h"
#include "monitor.h"
#include "softfloat/softfloat.h"
#include "utils/elfsym.h"

#define IMPL_CPU
#include "decode-cache.h"
#include "mmu-cache.h"

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
  return (tp.tv_sec * 1000000 + tp.tv_nsec / 1000) -
         nemu_start_time;
#elif 0
  struct rusage usage = {0};
  getrusage(RUSAGE_SELF /* current thread */, &usage);
  return (usage.ru_utime.tv_sec * 1000000 +
             usage.ru_utime.tv_usec) -
         nemu_start_time;
#endif
}

void kdbg_print_registers(void) {
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
    eprintf("$%s:0x%08x%c", regs[i], cpu.gpr[i],
        (i + 1) % 4 == 0 ? '\n' : ' ');
  }

  ninstr++;
}

static ALWAYS_INLINE uint32_t vaddr_read(
    vaddr_t addr, int len) {
  uint32_t idx = mmu_cache_index(addr);
  if (HAS_CONFIG(MMU_CACHE) &&
      mmu_cache[idx].id == mmu_cache_id(addr)) {
    ON_CONFIG(MMU_CACHE_PERF, mmu_cache_hit++);
    uint32_t data =
        *((uint32_t *)&mmu_cache[idx].ptr[addr & 0xFFF]) &
        (~0u >> ((4 - len) << 3));
    return data;
  } else {
    mmu_attr_t attr = {.rwbit = MMU_LOAD, .exbit = 1};
    paddr_t paddr = prot_addr_with_attr(addr, &attr);
    device_t *dev = find_device(paddr);
    CPUAssert(dev && dev->read, "bad addr %08x\n", addr);
    update_mmu_cache(addr, paddr, dev, attr.dirty);
    uint32_t data = dev->read(paddr - dev->start, len);
    return data;
  }
}

static ALWAYS_INLINE void vaddr_write(
    vaddr_t addr, int len, uint32_t data) {
  uint32_t idx = mmu_cache_index(addr);
  if (HAS_CONFIG(MMU_CACHE) &&
      mmu_cache[idx].id == mmu_cache_id(addr) &&
      mmu_cache[idx].can_write) {
    ON_CONFIG(MMU_CACHE_PERF, mmu_cache_hit++);
    memcpy(&mmu_cache[idx].ptr[addr & 0xFFF], &data, len);
  } else {
    paddr_t paddr = prot_addr(addr, MMU_STORE);
    device_t *dev = find_device(paddr);
    CPUAssert(dev && dev->write, "bad addr %08x\n", addr);
    update_mmu_cache(addr, paddr, dev, true);
    dev->write(paddr - dev->start, len, data);
  }
}

void raise_exception(uint32_t exception) {
  int code = exception & 0xFFFF;
  int extra = exception >> 16;

#if CONFIG_INSTR_LOG
  if (code == EXC_RI && get_current_instr() != 0x7c03e83b)
    printf("RI %08x %08x\n", cpu.pc, get_current_instr());
#endif

  if (code == EXC_TRAP) {
    panic("HIT BAD TRAP @%08x\n", get_current_pc());
  }

  cpu.has_exception = true;

  uint32_t vecOff = 0;
  if (HAS_CONFIG(MARCH_NOOP) || cpu.cp0.status.EXL == 0) {
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

  clear_mmu_cache();
  clear_decode_cache();
}

int init_cpu(vaddr_t entry) {
  nemu_start_time = get_current_time();

  cpu.cp0.count[0] = 0;
  cpu.cp0.compare = 0xFFFFFFFF;

#if !CONFIG_DIFF_WITH_QEMU
  cpu.cp0.status.CU = CU0_ENABLE;
#endif
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

  cpu.cp0.config1.MMU_size = NR_TLB_ENTRY - 1;

  /* initialize some cache */
  clear_mmu_cache();
  clear_decode_cache();

  /* prepare kernel commandline */
  extern const char *boot_cmdline;
  int len = boot_cmdline ? strlen(boot_cmdline) : 0;
  if (len > 0) {
    uint32_t cmdline_st = CONFIG_CMDLINE_ADDR + 8;
    /* argv[0] */
    vaddr_write(CONFIG_CMDLINE_ADDR, 4, cmdline_st);
    /* argv[1] */
    vaddr_write(CONFIG_CMDLINE_ADDR + 4, 4, 0);
    for (int i = 0; i < len; i++) {
      vaddr_write(cmdline_st + i, 1, boot_cmdline[i]);
    }
    vaddr_write(cmdline_st + len, 1, 0);
    cpu.gpr[R_a0] = 1;
    cpu.gpr[R_a1] = CONFIG_CMDLINE_ADDR;
  }

  return 0;
}

#if CONFIG_EXCEPTION || CONFIG_INTR
static ALWAYS_INLINE void check_intrs() {
#  if CONFIG_INTR
  cpu.cp0.count[0]++;
  if (cpu.cp0.count[0] == cpu.cp0.compare) {
    nemu_set_irq(7, 1);
  }
#  endif

  bool ie = !(cpu.cp0.status.ERL) &&
            !(cpu.cp0.status.EXL) && cpu.cp0.status.IE;
  if (ie && (cpu.cp0.status.IM & cpu.cp0.cause.IP)) {
    raise_exception(EXC_INTR);
  }
}
#endif

void nemu_epilogue() {
#if CONFIG_MMU_CACHE_PERF
  printf("mmu_cache: %lu/%lu = %lf\n", mmu_cache_hit,
      mmu_cache_hit + mmu_cache_miss,
      mmu_cache_hit /
          (double)(mmu_cache_hit + mmu_cache_miss));
#endif

#if CONFIG_DECODE_CACHE_PERF
  uint64_t decode_cache_total_hit =
      decode_cache_hit + decode_cache_fast_hit;
  uint64_t decode_cache_total =
      decode_cache_total_hit + decode_cache_miss;
  printf("decode_cache_hit: %lu/%lu = %lf\n",
      decode_cache_total_hit, decode_cache_total,
      decode_cache_total_hit / (double)decode_cache_total);
  printf("decode_cache_fast_hit: %lu/%lu = %lf\n",
      decode_cache_fast_hit, decode_cache_total,
      decode_cache_fast_hit / (double)decode_cache_total);
#endif

#if CONFIG_INSTR_LOG
  eprintf(">>>>>> last executed instructions\n");
  kdbg_print_instr_queue();
  eprintf("\n");
#endif

#if CONFIG_FUNCTION_TRACE_LOG
  eprintf(">>>>>> function invocations\n");
  kdbg_print_frames();
  eprintf("\n");
#endif

#if CONFIG_BACKTRACE_LOG
  eprintf(">>>>>> functions in stack\n");
  kdbg_print_backtraces();
  eprintf("\n");
#endif

#if CONFIG_INSTR_LOG
  eprintf(">>>>>> current registers\n");
  kdbg_print_registers();
  eprintf("\n");
#endif

#if CONFIG_ELF_PERF
  elfperf_report();
#endif

#if CONFIG_INSTR_PERF
  instrperf_report();
#endif
}

void nemu_exit() {
  nemu_epilogue();
  exit(0);
}

ALWAYS_INLINE const void *decoder_get_handler(Inst inst,
    const void *special_table[64],
    const void *special2_table[64],
    const void *special3_table[64],
    const void *bshfl_table[64],
    const void *regimm_table[64],
    const void *cop0_table_rs[32],
    const void *cop0_table_func[64],
    const void *cop1_table_rs[32],
    const void *cop1_table_rs_S[64],
    const void *cop1_table_rs_D[64],
    const void *cop1_table_rs_W[64],
    const void *opcode_table[64]) {
  const void *handler = NULL;
  switch (inst.op) {
  case 0x00: handler = special_table[inst.func]; break;
  case 0x01: handler = regimm_table[inst.rt]; break;
  case 0x10:
    if (inst.rs & 0x10)
      handler = cop0_table_func[inst.func];
    else
      handler = cop0_table_rs[inst.rs];
    break;
  case 0x11:
    switch (inst.rs) {
    case FPU_FMT_S:
      handler = cop1_table_rs_S[inst.func];
      break;
    case FPU_FMT_D:
      handler = cop1_table_rs_D[inst.func];
      break;
    case FPU_FMT_W:
      handler = cop1_table_rs_W[inst.func];
      break;
    default: handler = cop1_table_rs[inst.rs]; break;
    }
    break;
  case 0x1c: handler = special2_table[inst.func]; break;
  case 0x1f:
    if (inst.func == 0x20)
      handler = bshfl_table[inst.shamt];
    else
      handler = special3_table[inst.func];
    break;
  default: handler = opcode_table[inst.op]; break;
  }
  return handler;
}

ALWAYS_INLINE void decoder_set_state(
    decode_state_t *ds, Inst inst) {
  /* legacy code, keep it */
  switch (inst.op) {
  case 0x00: goto Rtype;
  case 0x01: goto Itype;
  case 0x02:
  case 0x03: goto Jtype;
  case 0x10:
    if (inst.rs & 0x10) {
      goto Handler;
    } else {
      goto Cop0Type;
    }
    break;
  case 0x1c: goto S2type;
  case 0x1f:
    if (inst.func == 0x20)
      goto bshflType;
    else
      goto S3type;
  default: goto Itype;
  }

  do {
  Rtype : {
    ds->rs = inst.rs;
    ds->rt = inst.rt;
    ds->rd = inst.rd;
    ds->shamt = inst.shamt;
    ds->func = inst.func;
    break;
  }
  Itype : {
    ds->rs = inst.rs;
    ds->rt = inst.rt;
    ds->uimm = inst.uimm;
    break;
  }
  Jtype : {
    ds->addr = inst.addr;
    break;
  }
  Cop0Type : {
    ds->rt = inst.rt;
    ds->rd = inst.rd;
    ds->sel = inst.sel;
    break;
  }
  S2type:;
  S3type : {
    ds->rs = inst.rs;
    ds->rt = inst.rt;
    ds->rd = inst.rd;
    ds->shamt = inst.shamt;
    break;
  }
  bshflType : {
    ds->rt = inst.rt;
    ds->rd = inst.rd;
    break;
  }
  } while (0);

Handler:
  return;
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
  ON_CONFIG(INSTR_LOG,
      bool nemu_needs_commit = work_mode == MODE_LOG);
  ON_CONFIG(INSTR_PERF, instrperf_start());
  ON_CONFIG(DECODE_CACHE,
      decode_state_t *ds = decode_cache_fetch(cpu.pc));

  for (; n > 0; n--) {
    /* local variables */
    Inst inst;

    ON_CONFIG(INSTR_LOG, instr_enqueue_pc(cpu.pc));
    ON_CONFIG(ELF_PERF, elfperf_record(cpu.pc));

#if CONFIG_EXCEPTION
    if ((cpu.pc & 0x3) != 0) {
      cpu.cp0.badvaddr = cpu.pc;
      raise_exception(EXC_AdEL);
      goto check_exception;
    }
#endif

    /* should be bad state */
#if CONFIG_WARN_PC_EQUALS_ZERO
    if (cpu.pc == 0x0) {
      printf("[NEMU] warning: cpu.pc == 0\n");
      kdbg_print_instr_queue();
    }
#endif

    /* concrete decoding and execution */
#include "exec/exec.h"

    ON_CONFIG(EXCEPTION, check_exception:);

#if CONFIG_INSTR_LOG
    if (nemu_needs_commit) kdbg_print_registers();
#endif

#if CONFIG_EXCEPTION || CONFIG_INTR
    if (!cpu.has_exception) check_intrs(); /* soft intr */

    if (cpu.has_exception) {
      cpu.has_exception = false;
      cpu.pc = cpu.br_target;
    }
#endif

    if (nemu_state != NEMU_RUNNING) { break; }
  }

  if (nemu_state == NEMU_RUNNING) {
    nemu_state = NEMU_STOP;
  }
}
