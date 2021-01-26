#include "exec-handlers.h"
#include "exec-macros.h"

/* clang-format on */
make_entry() {
  cpu.gpr[0] = 0;

#if CONFIG_DECODE_CACHE
  if (ds) {
    if (ds->next) {
      ds = ds->next;
#  if CONFIG_INSTR_LOG
      instr_enqueue_instr(ds->inst.val);
#  endif

      goto *(ds->handler);
    } else {
      ds->next = malloc(sizeof(decode_state_t));
      ds = ds->next;
      ds->next = NULL;
    }
  } else {
    ds = decode_cache_fetch(cpu.pc);
    if (ds->handler) {
#  if CONFIG_INSTR_LOG
      instr_enqueue_instr(ds->inst.val);
#  endif
      goto *(ds->handler);
    }
  }
#endif

  inst.val = vaddr_read(cpu.pc, 4);

#if CONFIG_INSTR_LOG
  instr_enqueue_instr(inst.val);
#if CONFIG_DECODE_CACHE
  ds->inst.val = inst.val;
#endif
#endif

  const void *handler = decoder_get_handler(inst,
      special_table, special2_table, special3_table,
      bshfl_table, regimm_table, cop0_table_rs,
      cop0_table_func, cop1_table_rs, cop1_table_rs_S,
      cop1_table_rs_D, cop1_table_rs_W, opcode_table);

#if CONFIG_DECODE_CACHE
  decoder_set_state(ds, inst);
  ds->handler = handler;
#endif
  goto *handler;
}

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
#if CONFIG_DECODE_CACHE
    ds = NULL;
#endif
    cpu.pc = cpu.br_target;
    cpu.is_delayslot = false;
  } else {
    cpu.pc += 4;
  }
  /* fall through */
}
#endif

make_exit() {}
