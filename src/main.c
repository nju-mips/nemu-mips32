#include <stdio.h>
#include <stdint.h>

int init_cpu();
int init_monitor(int, char *[]);
void gdb_mainloop(int);
void cpu_exec(uint64_t);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_cpu();
  int is_batch_mode = init_monitor(argc, argv);
  if(is_batch_mode) {
	cpu_exec(-1);
  } else {
	gdb_mainloop(is_batch_mode);
  }
  return 0;
}
