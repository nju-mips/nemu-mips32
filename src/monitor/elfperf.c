#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "utils/elfsym.h"
#include "utils/hash.h"

#if CONFIG_ELF_PERF_TIME
static const char *last_sym = NULL;
static uint64_t last_sym_st = 0;
#elif CONFIG_ELF_PERF_INSTR
static uint64_t ninstr = 0;
#endif

static hash_table_t perf_ht;

uint64_t get_current_time();

void elfperf_start() {
  elfsym_optimize_find_symbol(&elfsym);
}

void elfperf_record(uint32_t pc) {
  const char *sym = elfsym_find_symbol(&elfsym, pc);
  const char *updsym = sym;
#if CONFIG_ELF_PERF_TIME
  uint64_t ms = get_current_time();
  if (sym == last_sym) return;
  updsym = last_sym;
  last_sym = sym;
  if (!updsym) {
    last_sym_st = ms;
    return;
  }
#elif CONFIG_ELF_PERF_INSTR
  ninstr ++;
#endif

  hash_kv_t key = {(void *)updsym, strlen(updsym) + 1};
  hash_element_t *he = hash_get(&perf_ht, key);
  if (he == NULL) {
    uint64_t counter = CONFIG_IS_ENABLED(ELF_PERF_INSTR);
    hash_kv_t value = {&counter, sizeof(counter)};
    hash_push(&perf_ht, key, value);
  } else {
    assert(he->value.size == sizeof(uint64_t));
#if CONFIG_ELF_PERF_TIME
    (*(uint64_t *)he->value.buf) += ms - last_sym_st;
    last_sym_st = ms;
#elif CONFIG_ELF_PERF_INSTR
    (*(uint64_t *)he->value.buf)++;
#endif
  }
}

void elfperf_report() {
  for (int i = 0; i < HASH_SIZE; i++) {
    hash_element_t *he = perf_ht.pool[i];
    while (he) {
#if CONFIG_ELF_PERF
      const char *sym = he->key.buf;
      uint64_t *counter = he->value.buf;
#endif
#if CONFIG_ELF_PERF_TIME
      fprintf(stderr, "%40s\t%10ld.%06ld\n", sym,
          *counter / 1000000, *counter % 1000000);
#elif CONFIG_ELF_PERF_INSTR
      fprintf(stderr, "%40s\t%10ld\t%f%%\n", sym, *counter,
          100 * *counter / (float)ninstr);
#endif

      he = he->next;
    }
  }

  hash_free(&perf_ht);
}
