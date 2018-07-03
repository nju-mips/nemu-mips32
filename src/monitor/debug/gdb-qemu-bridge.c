#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "protocol.h"

extern char **environ;
void gdb_server_mainloop(int port);

void start_gdb(int port) {
  char remote_s[100];
  snprintf(remote_s, sizeof(remote_s), "target remote 127.0.0.1:%d", port);
  const char *argv[] = {
	"/usr/bin/gdb-multiarch",
	"-ex", remote_s,
	"-ex", "set arch mips",
	NULL,
  };
  execve(argv[0], (char **)argv, environ);
  assert(0);
}

void start_bridge(int port, int serv_port) {
  struct gdb_conn *client = gdb_server_start(port);
  struct gdb_conn *server = gdb_begin_inet("127.0.0.1", serv_port);

  size_t size = 0;
  char *data = NULL;
  while(1) {
	data = (void*)gdb_recv(client, &size);
	printf("$ message: client --> server:%lx:\n", size);
	printf("'%s'\n", data);
	printf("\n");
	gdb_send(server, (void*)data, size);
	free(data);

	data = (void*)gdb_recv(server, &size);
	printf("$ message: server --> client:%lx:\n", size);
	printf("'%s'\n", data);
	gdb_send(client, (void*)data, size);
	printf("\n\n");
	free(data);
  }
}

void test() {
  int serv_port = 1238;
  int gdb_port = serv_port + 1;
  if(fork() == 0) {
	 if(fork() == 0) {
	  gdb_server_mainloop(serv_port);
	 } else {
	  usleep(1000);
	  start_bridge(gdb_port, serv_port);
	 }
  } else {
	usleep(2000);
	start_gdb(gdb_port);
  }
}

