#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "elfsym.h"
#include "hash.h"

static uint64_t ninstr = 0;
static hash_table_t perf_ht;

void elfperf_start() {
  elfsym_optimize_find_symbol(&elfsym);
}

void elfperf_record(uint32_t pc) {
  const char *sym = elfsym_find_symbol(&elfsym, pc);
  hash_kv_t key = {(void *)sym, strlen(sym)};
  hash_element_t *he = hash_get(&perf_ht, key);
  ninstr++;
  if (he == NULL) {
    uint64_t counter = 1;
    hash_kv_t value = {&counter, sizeof(counter)};
    hash_push(&perf_ht, key, value);
  } else {
    assert(he->value.size == sizeof(uint64_t));
    (*(uint64_t *)he->value.buf)++;
  }
}

void elfperf_report() {
  for (int i = 0; i < HASH_SIZE; i++) {
    hash_element_t *he = perf_ht.pool[i];
    while (he) {
      const char *sym = he->key.buf;
      uint64_t *counter = he->value.buf;
      printf("%s\t%ld\t%.f\n", sym, *counter,
          (double)*counter / ninstr);

      he = he->next;
    }
  }

  hash_free(&perf_ht);
}
