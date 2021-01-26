#if CONFIG_INSTR_PERF
#  define make_label(l) \
  l:                    \
    instrperf_record(#l, sizeof(#l));
#else
#  define make_label(l) \
  l:
#endif
#define make_entry()
#define make_exit() make_label(exit)

#if CONFIG_EXCEPTION

#  define CHECK_ALIGNED_ADDR_AdEL(align, addr) \
    if (((addr) & (align - 1)) != 0) {         \
      cpu.cp0.badvaddr = addr;                 \
      raise_exception(EXC_AdEL);               \
      goto exit;                               \
    }

#  define CHECK_ALIGNED_ADDR_AdES(align, addr) \
    if (((addr) & (align - 1)) != 0) {         \
      cpu.cp0.badvaddr = addr;                 \
      raise_exception(EXC_AdES);               \
      goto exit;                               \
    }

#else

#  define CHECK_ALIGNED_ADDR(align, addr)                  \
    CPUAssert(((addr) & (align - 1)) == 0,                 \
        "address(0x%08x) is unaligned, pc=%08x\n", (addr), \
        cpu.pc)

#  define CHECK_ALIGNED_ADDR_AdEL CHECK_ALIGNED_ADDR
#  define CHECK_ALIGNED_ADDR_AdES CHECK_ALIGNED_ADDR

#endif

#if CONFIG_DELAYSLOT
#  define make_exec_handler(name) \
    goto inst_end;                \
    make_label(name)
#  define prepare_delayslot() \
    cpu.is_delayslot = true;  \
    cpu.pc += 4;              \
    goto exit;
#else
#  define make_exec_handler(name) \
    cpu.pc += 4;                  \
    goto exit;                    \
    make_label(name)
#  define prepare_delayslot() \
    cpu.pc = cpu.br_target;   \
    goto exit;
#endif

#if CONFIG_EXCEPTION
#  define InstAssert(cond)         \
    do {                           \
      if (!(cond)) {               \
        cpu.cp0.badvaddr = cpu.pc; \
        raise_exception(EXC_RI);   \
        goto exit;                 \
      }                            \
    } while (0)
#else
#  define InstAssert(cond) assert(cond)
#endif

#include "exec-handlers.h"

/* clang-format on */
make_entry() {
  cpu.gpr[0] = 0;

#if CONFIG_DECODE_CACHE_PERF
  decode_cache_hit += !!decode->handler;
  decode_cache_miss += !decode->handler;
#endif

#if CONFIG_DECODE_CACHE
  if (decode->handler) {
#  if CONFIG_INSTR_LOG
    instr_enqueue_instr(decode->inst.val);
#  endif

    goto *(decode->handler);
  }
#endif

  Inst inst = {.val = vaddr_read(cpu.pc, 4)};
#if CONFIG_INSTR_LOG
  instr_enqueue_instr(inst.val);
#endif

  const void *handler = decoder_get_handler(inst,
      special_table, special2_table, special3_table,
      bshfl_table, regimm_table, cop0_table_rs,
      cop0_table_func, cop1_table_rs, cop1_table_rs_S,
      cop1_table_rs_D, cop1_table_rs_W, opcode_table);

#if CONFIG_DECODE_CACHE
  decoder_set_state(decode, inst);
#endif

  decode->handler = handler;
  goto *handler;
}

#if CONFIG_DECODE_CACHE
#  define operands decode
#else
#  define operands (&inst)
#endif

#define GR_S operands->rs
#define GR_T operands->rt
#define GR_D operands->rd
#define GR_SV cpu.gpr[operands->rs]
#define GR_TV cpu.gpr[operands->rt]
#define GR_DV cpu.gpr[operands->rd]
#define I_SI operands->simm
#define I_UI operands->uimm
#define I_SA operands->shamt
#include "arith-ex.h"
#include "arith.h"
#include "branch.h"
#include "float.h"
#include "mdu.h"
#include "memory.h"
#include "setcc.h"
#include "shift.h"
#include "special.h"
#include "system.h"
#include "trap.h"

#if CONFIG_DELAYSLOT
make_label(inst_end) {
  if (cpu.is_delayslot) {
    cpu.pc = cpu.br_target;
    cpu.is_delayslot = false;
  } else {
    cpu.pc += 4;
  }
  /* fall through */
}
#endif

make_exit() {}
