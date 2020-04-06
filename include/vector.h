#ifndef VECTOR_H
#define VECTOR_H

#include <stdint.h>
#include <malloc.h>

typedef struct vec_t {
  size_t unit_size;
  size_t size;
  off_t ptr;
  void *p;
} vec_t;

/* init vector */
void vector_init(vec_t *v, size_t unit_size);

/* return a new pointer which point to new vector top */
void *vector_new(vec_t *v);

/* push a element into vector */
void vector_push(vec_t *v, void *t);

/* pop a element and return its pointer */
void *vector_pop(vec_t *v);

/*pop all elements*/
void vector_clear(vec_t *v);

/* return top element's pointer */
void *vector_top(vec_t *v);

/*return vector buffer*/
void *vector_ptr(vec_t *v);

/* resize the vector */
void vector_resize(vec_t *v, size_t size);

/* return the size */
size_t vector_size(vec_t *v);

/* free the memory which used by vector */
void vector_free(vec_t *v);

#endif
