#ifndef QUEUE_H
#define QUEUE_H

#define queue_type(type, len) \
  struct {                    \
    int f, r;                 \
    type e[len];              \
  }

#define queue_reset(q) \
  do { q.f = q.r = 0; } while (0)

#define queue_size(q) (sizeof(q.e) / sizeof(*q.e))

#define queue_push(q, ch)                 \
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

#define queue_for_each(q, ptr)                   \
  for (int i = (ptr = &q.e[q.f], q.f); i != q.r; \
       i = (i + 1) % queue_size(q), ptr = &q.e[i])

#define queue_top(q) (q.e[q.f])

#define queue_is_full(q) (q.f == (q.r + 1) % queue_size(q))

#define queue_is_empty(q) (q.f == q.r)

#endif
