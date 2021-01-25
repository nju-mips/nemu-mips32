#ifndef HASH_H
#define HASH_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define HASH_SIZE (4 * 1024)

typedef struct {
  void *buf;
  size_t size;
} hash_kv_t;

typedef struct hash_element_t {
  hash_kv_t key;
  hash_kv_t value;
  struct hash_element_t *next;
} hash_element_t;

typedef struct hash_table_t {
  hash_element_t *pool[HASH_SIZE];
} hash_table_t;

/* init a hash table */
void hash_init(hash_table_t *ht);
/* push a element into hash table */
void hash_push(
    hash_table_t *ht, hash_kv_t key, hash_kv_t value);
/* find element from hash table */
hash_element_t *hash_get(hash_table_t *ht, hash_kv_t key);
/*delete element from hash table*/
void hash_delete(hash_table_t *ht, hash_kv_t key);
/* destroy all elements in a hash table */
void hash_destroy_element(hash_table_t *ht);
/*free memory used by hash table*/
void hash_free(hash_table_t *ht);

#define hash_foreach(ht, he)                  \
  for (int _i = 0; _i < HASH_SIZE; _i++)      \
    for (he = ht.pool[_i]; he; he = he->next)

#endif
