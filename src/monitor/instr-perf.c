#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "utils/elfsym.h"
#include "utils/hash.h"

static hash_table_t instr_freq_table;

void instrperf_start() {
  hash_init(&instr_freq_table);
}

void instrperf_end() {
  hash_free(&instr_freq_table);
}

void instrperf_record(const char *instr, size_t size) {
  hash_kv_t key = {(void *)instr, size};
  hash_element_t *he = hash_get(&instr_freq_table, key);
  if (he) {
    (*(int *)(he->value.buf)) ++;
  } else {
    hash_kv_t key = {NULL, size};
    key.buf = strdup(instr);

    int *counter = malloc(sizeof(int));
    *counter = 0;
    hash_kv_t value = {counter, sizeof(int)};

    hash_push(&instr_freq_table, key, value);
  }
}

void instrperf_report() {
  hash_element_t *he = NULL;
  int64_t total = 0;
  hash_foreach(instr_freq_table, he) {
    total += *(int *)he->value.buf;
  }

  hash_foreach(instr_freq_table, he) {
    const char *s = he->key.buf;
    int c = *(int *)he->value.buf;
    printf("%.6f %10d %s\n", (float)c / total, c, s);
  }

  hash_free(&instr_freq_table);
}
