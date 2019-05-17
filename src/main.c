#include <stdio.h>
#include <stdint.h>
#include "monitor.h"

void init_mmio();
void init_device();
int init_monitor(int, char *[]);
void gdb_mainloop();
void qemu_diff();
void cpu_exec(uint64_t);

int main(int argc, char *argv[]) {
  init_mmio();
  /* Initialize the monitor. */
  work_mode_t mode = init_monitor(argc, argv);
  if(mode & MODE_BATCH) {
	init_device();
	if(mode == MODE_DIFF) {
	  qemu_diff();
	} else {
	  cpu_exec(-1);
	}
  } else {
	gdb_mainloop();
  }
  return 0;
}
