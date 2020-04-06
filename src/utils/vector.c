#include <time.h>
#include <string.h>

#include "debug.h"
#include "vector.h"

void vector_init(vec_t *v, size_t unit_size) {
  v->ptr = 0;
  v->unit_size = unit_size;
  v->size = 128 * v->unit_size;
  v->p = malloc(v->size);
}

void *vector_new(vec_t *v) {
  if (!v->size) return NULL;
  int oldptr = v->ptr;
  if (v->ptr >= v->size) {
    v->size *= 2;
    v->p = realloc(v->p, v->size);
  }
  v->ptr += v->unit_size;
  return (v->p + oldptr);
}

void vector_push(vec_t *v, void *t) {
  void *p = vector_new(v);
  memcpy(p, t, v->unit_size);
}

void *vector_pop(vec_t *v) {
  if (v->ptr > 0) {
    v->ptr -= v->unit_size;
    return (v->p + v->ptr);
  } else {
    return NULL;
  }
}

void vector_clear(vec_t *v) { v->ptr = 0; }

void *vector_ptr(vec_t *v) { return v->p; }

void *vector_top(vec_t *v) {
  if (v->ptr > 0) {
    return (v->p + v->ptr - v->unit_size);
  } else {
    return NULL;
  }
}

void vector_resize(vec_t *v, size_t size) {
  v->size = v->unit_size * size;
  v->p = realloc(v->p, v->size);
}

size_t vector_size(vec_t *v) {
  if (!(v->unit_size))
    panic("get size from a uninitialized vector.\n");
  return v->ptr / v->unit_size;
}

void vector_free(vec_t *v) {
  free(v->p);
  v->size = 0;
  v->ptr = 0;
}
