#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include "monitor/gdb/protocol.h"

extern char **environ;

void start_gdb() {
  const char *argv[] = {
	"/usr/bin/gdb-multiarch",
	"-ex",
	"target remote 127.0.0.1:3333",
	"-ex",
	"set arch mips",
	NULL,
  };
  execve(argv[0], (char **)argv, environ);
  assert(0);
}

void fork_server() {
  if(fork() != 0) return;

  // child process
  struct gdb_conn *conn = gdb_server_start(3333);
  while(1) {
	size_t size = 0;
	char *s = (void *)gdb_recv(conn, &size);
	printf("recv from host: %ld, '%s'\n", size, s);
  }
}

int init_cpu();
int init_monitor(int, char *[]);
void ui_mainloop(int);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
  init_cpu();

  // init_monitor(argc, argv);

  fork_server();
  start_gdb();

  /* Receive commands from user. */
  // ui_mainloop(is_batch_mode);

  return 0;
}
