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
#if CONFIG_INSTR_LOG
    instr_enqueue_instr(decode->inst.val);
#endif

    goto *(decode->handler);
  }
#endif

  decode.inst.val = vaddr_read(cpu.pc, 4);

#if CONFIG_INSTR_LOG
  instr_enqueue_instr(decode->inst.val);
#endif

  unsigned op = inst.op;
  switch (op) {
  case 0x00:
    decode->handler = special_table[inst.func];
    break;
  case 0x01: decode->handler = regimm_table[inst.rt]; break;
  case 0x10:
    if (inst.rs & 0x10)
      decode->handler = cop0_table_func[inst.func];
    else
      decode->handler = cop0_table_rs[inst.rs];
    break;
  case 0x11:
    switch (operands->rs) {
    case FPU_FMT_S:
      decode->handler = cop1_table_rs_S[operands->func];
      break;
    case FPU_FMT_D:
      decode->handler = cop1_table_rs_D[operands->func];
      break;
    case FPU_FMT_W:
      decode->handler = cop1_table_rs_W[operands->func];
      break;
    default:
      decode->handler = cop1_table_rs[operands->rs];
      break;
    }
    break;
  case 0x1c:
    decode->handler = special2_table[inst.func];
    break;
  case 0x1f:
    if (inst.func == 0x20)
      decode->handler = bshfl_table[inst.shamt];
    else
      decode->handler = special3_table[inst.func];
    break;
  default: decode->handler = opcode_table[op]; break;
  }

#if CONFIG_DECODE_CACHE
  /* notify compiler decode->handler is terminal handler */
  assert(decode->handler != &&exec_bshfl);
  assert(decode->handler != &&exec_special);
  assert(decode->handler != &&exec_regimm);
  assert(decode->handler != &&exec_cop0);
  assert(decode->handler != &&exec_cop1);
  assert(decode->handler != &&exec_special2);
  assert(decode->handler != &&exec_special3);

  switch (op) {
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
      goto Handler;
  default: goto Itype;
  }

  do {
  Rtype : {
    decode->rs = inst.rs;
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->shamt = inst.shamt;
    decode->func = inst.func;
    break;
  }
  Itype : {
    decode->rs = inst.rs;
    decode->rt = inst.rt;
    decode->uimm = inst.uimm;
    break;
  }
  Jtype : {
    decode->addr = inst.addr;
    break;
  }
  Cop0Type : {
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->sel = inst.sel;
    break;
  }
  S2type : {
    decode->rs = inst.rs;
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->shamt = inst.shamt;
    break;
  }
  bshflType : {
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    break;
  }
  } while (0);
#endif

Handler:
  goto *(decode->handler);
}

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
