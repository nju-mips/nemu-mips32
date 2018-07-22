#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

extern void print_registers();

#define eprintf(...) fprintf(stderr, ## __VA_ARGS__)

// we are properly doing diff testing in batch_mode, so do not Log in batch_mode
#define Log(format, ...) \
  do { \
      printf("\e[1;34m[%s,%d,%s] " format "\e[0m\n", \
          __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
  } while (0)

#define Assert(cond, fmt, ...) do { \
    if (!(cond)) { \
	  eprintf("nemu: %s:%d: %s: Assertion `%s' failed\n", \
			  __FILE__, __LINE__, __func__, #cond); \
      eprintf("\33[1;31m" fmt "\33[0m\n", ## __VA_ARGS__); \
	  abort(); \
    } \
  } while (0)

#define CPUAssert(cond, fmt, ...) do { \
    if (!(cond)) { \
	  eprintf("nemu: %s:%d: %s: Assertion `%s' failed\n", \
			  __FILE__, __LINE__, __func__, #cond); \
	  eprintf("=========== dump registers =========\n"); \
	  print_registers(); \
	  eprintf("=========== dump    end =========\n"); \
      eprintf("\33[1;31m" fmt "\33[0m\n", ## __VA_ARGS__); \
	  abort(); \
    } \
  } while (0)

#define panic(fmt, ...) do { \
	eprintf("nemu: %s:%d: %s: panic: \e[1;31m" fmt "\e[0m\n", \
			__FILE__, __LINE__, __func__, ## __VA_ARGS__); \
	abort(); \
  } while(0)

#define TODO() panic("please implement me")

#endif
