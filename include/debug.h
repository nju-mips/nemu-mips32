#ifndef DEBUG_H
#define DEBUG_H

#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "monitor.h"

extern void check_kernel_image(const char *image);

extern void instr_enqueue_pc(uint32_t pc);
extern void instr_enqueue_instr(uint32_t pc);
extern void print_instr_queue(void);
extern void print_registers(void);

#define eprintf(...) fprintf(stderr, ##__VA_ARGS__)

#define Abort() exit(-1)

// we are properly doing diff testing in batch_mode, so do
// not Log in batch_mode
#define Log(format, ...)                          \
  do {                                            \
    eprintf("nemu: %s:%d: %s: " format, __FILE__, \
            __LINE__, __func__, ##__VA_ARGS__);   \
  } while (0)

#define Assert(cond, fmt, ...)                             \
  do {                                                     \
    if (!(cond)) {                                         \
      Log("Assertion `%s' failed: \e[1;31m" fmt "\e[0m\n", \
          #cond, ##__VA_ARGS__);                           \
      Abort();                                             \
    }                                                      \
  } while (0)

#define panic(fmt, ...)                                   \
  do {                                                    \
    eprintf("nemu: %s:%d: %s: panic: \e[1;31m" fmt        \
            "\e[0m\n",                                    \
            __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    Abort();                                              \
  } while (0)

#define TODO() panic("please implement me")

#define CPUAbort()                      \
  do {                                  \
    extern jmp_buf gdb_mode_top_caller; \
    if (work_mode == MODE_GDB) {        \
      longjmp(gdb_mode_top_caller, 1);  \
    } else {                            \
      abort();                          \
    }                                   \
  } while (0)

#define CPUAssert(cond, fmt, ...)                         \
  do {                                                    \
    if (!(cond)) {                                        \
      eprintf("nemu: %s:%d: %s: Assertion `%s' failed\n", \
              __FILE__, __LINE__, __func__, #cond);       \
      print_instr_queue();                                \
      eprintf("=========== dump registers =========\n");  \
      print_registers();                                  \
      eprintf("=========== dump    end =========\n");     \
      eprintf("\e[1;31m" fmt "\e[0m\n", ##__VA_ARGS__);   \
      CPUAbort();                                         \
    }                                                     \
  } while (0)

#endif
