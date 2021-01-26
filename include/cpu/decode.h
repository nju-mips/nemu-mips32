#ifndef CPU_DECODE_CACHE_H
#define CPU_DECODE_CACHE_H

/* clang-format off */
typedef struct {
  const void *handler;

#if CONFIG_DECODE_CACHE
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
#else
  Inst inst;
#endif
} decode_state_t;
/* clang-format on */

#endif
