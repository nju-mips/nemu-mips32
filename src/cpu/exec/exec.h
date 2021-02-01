#include "exec-handlers.h"
#include "exec-macros.h"

/* clang-format on */
make_entry() {
  cpu.gpr[0] = 0;

#if CONFIG_DECODE_CACHE
  if (ds->next) {
    ds = ds->next;
    ON_CONFIG(INSTR_LOG, instr_enqueue_instr(ds->inst.val));
    ON_CONFIG(DECODE_CACHE_PERF, decode_cache_fast_hit++);

    goto *(ds->handler);
  } else {
    ON_CONFIG(DECODE_CACHE_PERF, decode_cache_miss++);

    switch (ds->itype) {
    case IT_COM:
      ds->next = decode_cache_fetch(cpu.pc);
      ds = ds->next;
      ds->next = NULL;
      break;
    case IT_BR_T: ds = ds->true_next; break;
    case IT_BR_F: ds = ds->false_next; break;
    case IT_J: ds = ds->j_next; break;
    case IT_JR: ds = ds->jr_next; break;
    case IT_DS: ds = ds->ds_next; break;
    default: assert(0); break;
    }

    if (ds->handler) {
      ON_CONFIG(
          INSTR_LOG, instr_enqueue_instr(ds->inst.val));
      goto *(ds->handler);
    }
  }
#endif

  inst.val = vaddr_read(cpu.pc, 4);

#if CONFIG_INSTR_LOG
  instr_enqueue_instr(inst.val);
  ON_CONFIG(DECODE_CACHE, ds->inst = inst);
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
    cpu.pc = cpu.br_target;
    cpu.is_delayslot = false;
  } else {
    cpu.pc += 4;
  }
  /* fall through */
}
#endif

make_exit() {}
