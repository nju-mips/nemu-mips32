#include "common.h"
#include "utils.h"
#include <sys/time.h>

struct frame_t {
  enum { NONE, CALL, RET } property;
  uint32_t pc;
  uint32_t target;
  struct timeval time;
};

static int pc_ptr = 0;
static struct frame_t frames[4000000];

#define NR_FRAMES (sizeof(frames) / sizeof(frames[0]))

void frames_enqueue_call(vaddr_t pc, vaddr_t target) {
  frames[pc_ptr].property = CALL;
  frames[pc_ptr].pc = pc;
  frames[pc_ptr].target = target;
  gettimeofday(&frames[pc_ptr].time, NULL);
  pc_ptr = (pc_ptr + 1) % NR_FRAMES;
}

void frames_enqueue_ret(vaddr_t pc, vaddr_t target) {
  frames[pc_ptr].property = RET;
  frames[pc_ptr].pc = pc;
  frames[pc_ptr].target = target;
  gettimeofday(&frames[pc_ptr].time, NULL);
  pc_ptr = (pc_ptr + 1) % NR_FRAMES;
}

static void prepare_symbols() {
  static bool prepared = false;
  if (!prepared) {
    extern const char *symbol_file;
    load_elf_symtab(symbol_file);
  }
}

void print_frames(void) {
  prepare_symbols();

  eprintf("last collected %ld frames:\n", NR_FRAMES);
  int i = pc_ptr;
  do {
    if (frames[i].property == CALL)
      eprintf("%02ld.%06ld: %08x CALL   %08x: %-20s -> %-20s\n",
          frames[i].time.tv_sec, frames[i].time.tv_usec, frames[i].pc,
          frames[i].target, find_symbol_by_addr(frames[i].pc),
          find_symbol_by_addr(frames[i].target));
    else if (frames[i].property == RET)
      eprintf("%02ld.%06ld: %08x RET TO %08x: %-20s -> %-20s\n",
          frames[i].time.tv_sec, frames[i].time.tv_usec, frames[i].pc,
          frames[i].target, find_symbol_by_addr(frames[i].pc),
          find_symbol_by_addr(frames[i].target));
    else
      /* eprintf("XXXXXXXX: NONE   xxxxxxxx\n")*/;
    i = (i + 1) % NR_FRAMES;
  } while (i != pc_ptr);
}

void print_backtrace() {
  prepare_symbols();

#define NR_BACKTRACE 100
  static uint32_t backtraces[NR_BACKTRACE];
  uint32_t top = 0;
  int i = pc_ptr;
  memset(backtraces, -1, sizeof(backtraces));
  do {
    if (frames[i].property == CALL) {
      backtraces[top++] = i;
      assert(top < NR_BACKTRACE);
    } else if (frames[i].property == RET) {
      if (top > 0) top--;
    }
    i = (i + 1) % NR_FRAMES;
  } while (i != pc_ptr);

  eprintf("last collected %ld backtraces:\n", NR_FRAMES);
  for (int i = 0; i < top; i++) {
    int idx = backtraces[i];
    if (idx < 0) continue;
    assert(idx < NR_FRAMES);
    eprintf("%08x:%08x %-20s -> %-20s\n", frames[idx].pc, frames[idx].target,
        find_symbol_by_addr(frames[idx].pc),
        find_symbol_by_addr(frames[idx].target));
  }
}
