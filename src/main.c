#include <stdio.h>
#include <stdint.h>

void init_device();
int init_monitor(int, char *[]);
void gdb_mainloop();
void cpu_exec(uint64_t);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  int is_batch_mode = init_monitor(argc, argv);
  if(is_batch_mode) {
	init_device();
	cpu_exec(-1);
  } else {
	gdb_mainloop();
  }
  return 0;
}
