#ifndef CPU_DECODE_CACHE_H
#define CPU_DECODE_CACHE_H

#include "reg.h"

/* clang-format off */
struct decode_state_t {
  const void *handler;
  struct decode_state_t *next;

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

  struct decode_state_t *j_target;
  int sel; /* put here will improve performance */

  // FIXME
  union {
    int nd, tf, cc1, cc2;
    int fs64, fd64, ft64;
  };
#if CONFIG_INSTR_LOG
  Inst inst;
#endif
};

typedef struct decode_state_t decode_state_t;
/* clang-format on */

#endif
