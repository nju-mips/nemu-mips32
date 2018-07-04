#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <assert.h>


// we are properly doing diff testing in batch_mode, so do not Log in batch_mode
#define Log(format, ...) \
  do { \
    extern int is_batch_mode; \
    if (!is_batch_mode) { \
      printf("\e[1;34m[%s,%d,%s] " format "\e[0m\n", \
          __FILE__, __LINE__, __func__, ## __VA_ARGS__); \
    } \
  } while (0)

#define Assert(cond, ...) \
  do { \
    if (!(cond)) { \
      fflush(stdout); \
      fprintf(stderr, "\33[1;31m"); \
      fprintf(stderr, __VA_ARGS__); \
      fprintf(stderr, "\33[0m\n"); \
      assert(cond); \
    } \
  } while (0)

#define panic(format, ...) \
  Assert(0, format, ## __VA_ARGS__)

#define TODO() panic("please implement me")

#endif
