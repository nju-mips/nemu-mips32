#ifndef IMPL_CPU
#  error "this file can only be included by cpu.c"
#endif

#if CONFIG_DECODE_CACHE_PERF
uint64_t decode_cache_fast_hit = 0;
uint64_t decode_cache_hit = 0;
uint64_t decode_cache_miss = 0;
#endif

#define DECODE_CACHE_BITS 12

static decode_state_t *decode_cache[1 << DECODE_CACHE_BITS];

void clear_decode_cache() { /* FIXME: */
}

static ALWAYS_INLINE uint32_t decode_cache_index(
    vaddr_t vaddr) {
  return (vaddr >> 2) & ((1 << DECODE_CACHE_BITS) - 1);
}

static ALWAYS_INLINE decode_state_t *decode_cache_fetch(
    vaddr_t pc) {
  uint32_t idx = decode_cache_index(pc);
  decode_state_t *ds = decode_cache[idx];
  while (ds && ds->pc != pc) ds = ds->l_next;
  if (ds) return ds;

  ds = malloc(sizeof(decode_state_t));
  *ds = (decode_state_t){};
  ds->pc = pc;
  ds->l_next = decode_cache[idx];
  decode_cache[idx] = ds;
  return ds;
}

// delayslot
static ALWAYS_INLINE decode_state_t *ds_set_ds(
    decode_state_t *ds, uint32_t pc) {
  if (ds->next) return ds->next;

  ds->itype = IT_DS;
  ds->ds_next = decode_cache_fetch(cpu.pc + 4);
  return ds->ds_next;
}

static void ALWAYS_INLINE ds_set_br_true(
    decode_state_t *ds, uint32_t pc) {
  ON_CONFIG(DELAYSLOT, ds = ds_set_ds(ds, pc));
  if (ds->true_next)
    ds->next = ds->true_next;
  else {
    ds->true_next = decode_cache_fetch(pc);
    ds->next = NULL;
    ds->itype = IT_BR_T;
  }
}

static void ALWAYS_INLINE ds_set_br_false(
    decode_state_t *ds, uint32_t pc) {
  ON_CONFIG(DELAYSLOT, ds = ds_set_ds(ds, pc));
  if (ds->false_next)
    ds->next = ds->false_next;
  else {
    ds->false_next = decode_cache_fetch(pc);
    ds->next = NULL;
    ds->itype = IT_BR_F;
  }
}

static void ALWAYS_INLINE ds_set_j(
    decode_state_t *ds, uint32_t pc) {
  ON_CONFIG(DELAYSLOT, ds = ds_set_ds(ds, pc));
  if (ds->j_next)
    ds->next = ds->j_next;
  else {
    ds->j_next = decode_cache_fetch(pc);
    ds->itype = IT_J;
  }
}

static void ALWAYS_INLINE ds_set_jr(
    decode_state_t *ds, uint32_t pc) {
  ON_CONFIG(DELAYSLOT, ds = ds_set_ds(ds, pc));
  if (ds->jr_next && ds->jr_next->pc == pc)
    ds->next = ds->jr_next;
  else {
    ds->jr_next = decode_cache_fetch(pc);
    ds->itype = IT_JR;
    ds->next = NULL;
  }
}
