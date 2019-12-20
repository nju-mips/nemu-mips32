#ifndef QUEUE_H
#define QUEUE_H

#define queue_type(type, len) \
  struct {                    \
    int f, r;                 \
    type e[len];              \
  }

#define queue_size(q) (sizeof(q.e) / sizeof(*q.e))

#define queue_add(q, ch)                  \
  do {                                    \
    int next = (q.r + 1) % queue_size(q); \
    if (next != q.f) {                    \
      q.e[q.r] = ch;                      \
      q.r = next;                         \
    }                                     \
  } while (0)

#define queue_pop(q)                                 \
  ({                                                 \
    int data = q.e[q.f];                             \
    if (q.f != q.r) q.f = (q.f + 1) % queue_size(q); \
    data;                                            \
  })

#define queue_top(q) (q.e[q.f]);

#define queue_is_full(q) (q.f != (q.r + 1) % queue_size(q))

#define queue_is_empty(q) (q.f == q.r)

#endif
