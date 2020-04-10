#include <stdint.h>
#include <stdio.h>

#include "monitor.h"

void init_mmio();
void init_events();
void parse_args(int, char *[]);
int init_monitor(void);
void gdb_mainloop();
void difftest();
void cpu_exec(uint64_t);

int main(int argc, char *argv[]) {
  parse_args(argc, argv);
  init_mmio();

  /* Initialize the monitor. */
  work_mode_t mode = init_monitor();
  if (mode & MODE_BATCH) {
    init_events();
    if (mode == MODE_DIFF) {
      difftest();
    } else {
      cpu_exec(-1);
      nemu_exit(0);
    }
  } else {
    gdb_mainloop();
  }
  return 0;
}
