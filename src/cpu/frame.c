#include "common.h"

struct frame_t {
  enum { NONE, CALL, RET } property;
  uint32_t pc;
  uint32_t target;
};

static int pc_ptr = 0;
static struct frame_t frames[400];

#define NR_FRAMES (sizeof(frames) / sizeof(frames[0]))

void frames_enqueue_call(vaddr_t pc, vaddr_t target) {
  frames[pc_ptr].property = CALL;
  frames[pc_ptr].pc = pc;
  frames[pc_ptr].target = target;
  pc_ptr = (pc_ptr + 1) % NR_FRAMES;
}

void frames_enqueue_ret(vaddr_t pc, vaddr_t target) {
  frames[pc_ptr].property = RET;
  frames[pc_ptr].pc = pc;
  frames[pc_ptr].target = target;
  pc_ptr = (pc_ptr + 1) % NR_FRAMES;
}

void print_frames(void) {
  eprintf("last collected %ld frames:\n", NR_FRAMES);
  int i = pc_ptr;
  do {
	if(frames[i].property == CALL)
	  eprintf("%08x: CALL   %08x\n", frames[i].pc, frames[i].target);
    else if(frames[i].property == RET)
	  eprintf("%08x: RET TO %08x\n", frames[i].pc, frames[i].target);
	else
	  eprintf("XXXXXXXX: NONE   xxxxxxxx\n");
	i = (i + 1) % NR_FRAMES;
  } while(i != pc_ptr);
}
