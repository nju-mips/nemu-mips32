#ifndef FIFO_H
#define FIFO_H

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>

#define fifo_type(type, len) \
  struct {                   \
    unsigned head;           \
    unsigned size;           \
    pthread_mutex_t mut;     \
    type e[len];             \
  }

#define fifo_init(q) pthread_mutex_init(&q.mut, NULL)

#define fifo_reset(q)             \
  do {                            \
    pthread_mutex_lock(&q.mut);   \
    q.head = q.size = 0;          \
    pthread_mutex_unlock(&q.mut); \
  } while (0)

#define fifo_size(q) (q.size)

#define fifo_capacity(q) (sizeof(q.e) / sizeof(*q.e))

#define fifo_push(q, ch)                                  \
  do {                                                    \
    pthread_mutex_lock(&q.mut);                           \
    unsigned next = (q.head + q.size) % fifo_capacity(q); \
    if (q.size < fifo_capacity(q)) {                      \
      q.e[next] = ch;                                     \
      q.size++;                                           \
    }                                                     \
    pthread_mutex_unlock(&q.mut);                         \
  } while (0)

#define fifo_pop(q)                             \
  ({                                            \
    pthread_mutex_lock(&q.mut);                 \
    unsigned old_h = q.head;                    \
    if (q.size > 0) {                           \
      q.head = (q.head + 1) % fifo_capacity(q); \
      q.size--;                                 \
    }                                           \
    pthread_mutex_unlock(&q.mut);               \
    q.e[old_h];                                 \
  })

#define fifo_foreach(q, ptr)                             \
  for (unsigned i = (ptr = &q.e[q.head], 0); i < q.size; \
       i++, ptr = &q.e[(q.head + i) % fifo_capacity(q)])

#define fifo_top(q) (q.e[q.head])

#define fifo_is_full(q) (q.size == fifo_capacity(q))

#define fifo_is_empty(q) (q.size == 0)

#endif
