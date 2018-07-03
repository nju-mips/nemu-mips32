#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "protocol.h"

extern char **environ;

void start_gdb() {
  const char *argv[] = {
	"/usr/bin/gdb",
	"-ex", "target remote 127.0.0.1:3333",
	NULL,
  };
  execve(argv[0], (char **)argv, environ);
  assert(0);
}

void start_server() {
  if(fork() != 0) return;

  // child process
  struct gdb_conn *gdb = gdb_server_start(3333);
  struct gdb_conn *qemu = gdb_begin_inet("127.0.0.1", 1234);

  size_t size = 0;
  char *data = NULL;
  while(1) {
	data = (void*)gdb_recv(gdb, &size);
	printf("$ message: gdb --> qemu:%ld:\n", size);
	printf("'%s'\n", data);
	printf("\n");
	gdb_send(qemu, (void*)data, size);
	free(data);

	data = (void*)gdb_recv(qemu, &size);
	printf("$ message: qemu --> gdb:%ld:\n", size);
	printf("'%s'\n", data);
	gdb_send(gdb, (void*)data, size);
	printf("\n\n");
	free(data);
  }
}

void start_gdb_qemu_bridge() {
  start_server();
  start_gdb();
}
