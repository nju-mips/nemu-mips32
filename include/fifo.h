#ifndef FIFO_H
#define FIFO_H

#include <stddef.h>
#include <stdio.h>

#define fifo_type(type, len) \
  struct {                   \
    unsigned head;           \
    unsigned size;           \
    type e[len];             \
  }

#define fifo_reset(q) \
  do { q.head = q.size = 0; } while (0)

#define fifo_size(q) (q.size)

#define fifo_capacity(q) (sizeof(q.e) / sizeof(*q.e))

#define fifo_push(q, ch)                                  \
  do {                                                    \
    unsigned next = (q.head + q.size) % fifo_capacity(q); \
    if (q.size < fifo_capacity(q)) {                      \
      q.e[next] = ch;                                     \
      q.size++;                                           \
    }                                                     \
  } while (0)

#define fifo_pop(q)                             \
  ({                                            \
    int data = q.e[q.head];                     \
    if (q.size > 0) {                           \
      q.head = (q.head + 1) % fifo_capacity(q); \
      q.size--;                                 \
    }                                           \
    data;                                       \
  })

#define fifo_foreach(q, ptr)                             \
  for (unsigned i = (ptr = &q.e[q.head], 0); i < q.size; \
       i++, ptr = &q.e[(q.head + i) % fifo_capacity(q)])

#define fifo_top(q) (q.e[(q.head + q.size) % fifo_capacity(q)])

#define fifo_is_full(q) (q.size == fifo_capacity(q))

#define fifo_is_empty(q) (q.size == 0)

#endif
