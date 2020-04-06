#include <string.h>

#include "hash.h"

typedef int (*hash_func_t)(const uint8_t *, size_t);

hash_func_t hash_func = NULL;

int bkdr_hash(const uint8_t *keybuf, size_t size) {
  int h = 0;
  for (int i = 0; i < size; i++) {
    h = h * 131 + keybuf[i];
  }
  return (h & 0x7FFFFFFF) % HASH_SIZE;
}

void hash_init(hash_table_t *ht) {
  memset(ht, 0, sizeof(hash_table_t));
  vector_init(&(ht->full), sizeof(int));
}

void hash_push(hash_table_t *ht, const void *keybuf,
    size_t size, const void *value) {
  int key = hash_func(keybuf, size);
  if (!ht->pool[key]) vector_push(&(ht->full), &key);

  hash_element_t *he =
      (hash_element_t *)malloc(sizeof(hash_element_t));
  memset(he, 0, sizeof(*he));

  he->keybuf = (void *)keybuf;
  he->size = size;
  he->next = ht->pool[key];
  he->value = (void *)value;
  ht->pool[key] = he;
  /**/
}

void *hash_get(
    hash_table_t *ht, const void *keybuf, size_t size) {
  int key = hash_func(keybuf, size);
  hash_element_t *he = ht->pool[key];
  while (he) {
    if (he->size == size &&
        memcmp(he->keybuf, keybuf, size) == 0) {
      return he->value;
    }
    he = he->next;
  }
  return NULL;
}

void hash_delete(
    hash_table_t *ht, const void *keybuf, size_t size) {
  int key = hash_func(keybuf, size);
  hash_element_t *he = ht->pool[key];
  hash_element_t *phe = NULL;
  while (he) {
    if (he->size == size &&
        memcmp(he->keybuf, keybuf, size) == 0) {
      // logd("hash delete: %s\n", (char *)keybuf);
      if (phe)
        phe->next = he->next;
      else
        ht->pool[key] = he->next;
      break;
    }
    phe = he;
    he = he->next;
  }
}

static void free_hash_slot(hash_element_t *he) {
  while (he) {
    hash_element_t *curhe = he;
    he = he->next;
    free(curhe);
  }
}

/*use vector full to record how many slots are in use*/
void hash_destroy_element(hash_table_t *ht) {
  size_t vec_n = vector_size(&(ht->full));
  if (vec_n * 4 > HASH_SIZE) {
    for (int i = 0; i < HASH_SIZE; i++)
      free_hash_slot(ht->pool[i]);
    memset(&(ht->pool), 0, sizeof(ht->pool));
  } else {
    int *p = ht->full.p;
    for (int i = 0; i < vec_n; i++) {
      free_hash_slot(ht->pool[p[i]]);
      ht->pool[p[i]] = NULL;
    }
  }

  vector_clear(&(ht->full));
}

void hash_free(hash_table_t *ht) {
  hash_destroy_element(ht);
  vector_free(&(ht->full));
}
