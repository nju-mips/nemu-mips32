#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/hash.h"

static uint32_t bkdr_hash(hash_kv_t key) {
  uint32_t h = 0;
  for (int i = 0; i < key.size; i++) {
    h = h * 131 + ((uint8_t *)key.buf)[i];
  }
  return (h & 0x7FFFFFFF) % HASH_SIZE;
}

static hash_kv_t hash_kv_copy(hash_kv_t kv) {
  hash_kv_t copy = {};
  copy.size = kv.size;
  copy.buf = malloc(copy.size);
  memcpy(copy.buf, kv.buf, copy.size);
  return copy;
}

static void hash_kv_release(hash_kv_t kv) { free(kv.buf); }

static bool hash_kv_equals(hash_kv_t a, hash_kv_t b) {
  if (a.size != b.size) return false;
  return memcmp(a.buf, b.buf, a.size) == 0;
}

void hash_init(hash_table_t *ht) {
  memset(ht, 0, sizeof(hash_table_t));
}

void hash_push(
    hash_table_t *ht, hash_kv_t key, hash_kv_t value) {
  int index = bkdr_hash(key);
  hash_element_t *he =
      (hash_element_t *)malloc(sizeof(hash_element_t));

  he->key = hash_kv_copy(key);
  he->next = ht->pool[index];
  he->value = hash_kv_copy(value);
  ht->pool[index] = he;
}

hash_element_t *hash_get(hash_table_t *ht, hash_kv_t key) {
  int index = bkdr_hash(key);
  hash_element_t *he = ht->pool[index];
  while (he) {
    if (hash_kv_equals(he->key, key)) { return he; }
    he = he->next;
  }
  return NULL;
}

void hash_delete(hash_table_t *ht, hash_kv_t key) {
  int index = bkdr_hash(key);
  hash_element_t *he = ht->pool[index];
  hash_element_t *phe = NULL;
  while (he) {
    if (hash_kv_equals(he->key, key)) {
      if (phe)
        phe->next = he->next;
      else
        ht->pool[index] = he->next;
      break;
    }
    phe = he;
    he = he->next;
  }
}

void free_hash_slot(hash_element_t *he) {
  while (he) {
    hash_element_t *curhe = he;
    he = he->next;

    hash_kv_release(curhe->key);
    hash_kv_release(curhe->value);
    free(curhe);
  }
}

/*use vector full to record how many slots are in use*/
void hash_free(hash_table_t *ht) {
  for (int i = 0; i < HASH_SIZE; i++)
    free_hash_slot(ht->pool[i]);
  memset(&(ht->pool), 0, sizeof(ht->pool));
}
