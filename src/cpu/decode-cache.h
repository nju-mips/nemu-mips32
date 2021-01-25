#ifndef IMPL_CPU
#  error "this file can only be included by cpu.c"
#endif

#if CONFIG_DECODE_CACHE_PERF
uint64_t decode_cache_hit = 0;
uint64_t decode_cache_miss = 0;
#endif

#define DECODE_CACHE_BITS 12
/* clang-format off */
typedef struct {
  const void *handler;
  uint32_t id;
  union {
    struct {
      int rs;
      union { int rt; int ft; }; // R and I

      union {
        struct {
          union { int rd; int fs; };
          union { int shamt; int fd; };
          int func;
        }; // R

        union {
          uint16_t uimm;
          int16_t simm;
        }; // I
      };   // R and I union
    };     // R and I union

    uint32_t addr; // J
  };

  int sel; /* put here will improve performance */

  int nd, tf, cc1, cc2;
  int fs64, fd64, ft64;

#if CONFIG_INSTR_LOG || CONFIG_DECODE_CACHE_CHECK
  Inst inst;
#endif
} decode_cache_t;
/* clang-format on */

static decode_cache_t decode_cache[1 << DECODE_CACHE_BITS];

void clear_decode_cache() {
  for (int i = 0;
       i < sizeof(decode_cache) / sizeof(*decode_cache);
       i++) {
    decode_cache[i].handler = NULL;
  }
}

static ALWAYS_INLINE uint32_t decode_cache_index(
    vaddr_t vaddr) {
  return vaddr & ((1 << DECODE_CACHE_BITS) - 1);
}

static ALWAYS_INLINE uint32_t decode_cache_id(
    vaddr_t vaddr) {
  return (vaddr >> DECODE_CACHE_BITS);
}

static ALWAYS_INLINE decode_cache_t *decode_cache_fetch(
    vaddr_t pc) {
  uint32_t idx = decode_cache_index(pc);
  uint32_t id = decode_cache_id(pc);
  if (decode_cache[idx].id != id) {
    decode_cache[idx].handler = NULL;
    decode_cache[idx].id = id;
  }
  return &decode_cache[idx];
}
