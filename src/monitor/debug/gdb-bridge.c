#include <arpa/inet.h>
#include <assert.h>
#include <malloc.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "debug.h"
#include "protocol.h"

extern char *elf_file;
extern char **environ;

void init_device();
void gdb_server_mainloop(int port);

void start_gdb(int port) {
  char symbol_s[100], remote_s[100];

  snprintf(symbol_s, sizeof(symbol_s), "symbol %s", elf_file);
  snprintf(remote_s, sizeof(remote_s),
		  "target remote 127.0.0.1:%d", port);
  const char *argv[] = {
#ifdef ON_QEMU
	"/usr/bin/gdb",
#else
	"/usr/bin/gdb-multiarch",
	"-ex", "set arch mips",
	"-ex", symbol_s,
#endif
	"-ex", remote_s,
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

int get_free_servfd() {
  // fill the socket information
  struct sockaddr_in sa = {
    .sin_family = AF_INET,
    .sin_port = 0,
	.sin_addr.s_addr = htonl(INADDR_ANY),
  };

  // open the socket and start the tcp connection
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if(bind(fd, (const struct sockaddr *)&sa, sizeof(sa)) != 0) {
	close(fd);
	panic("bind");
  }
  return fd;
}

int get_port_of_servfd(int fd) {
  struct sockaddr_in serv_addr;
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = 0;

  socklen_t len = sizeof(serv_addr);
  if (getsockname(fd, (struct sockaddr *)&serv_addr, &len) == -1) {
	  perror("getsockname");
	  return -1;
  }
  return ntohs(serv_addr.sin_port);
}

void gdb_mainloop() {
  int servfd = get_free_servfd();
  int port = get_port_of_servfd(servfd);

  if(fork() == 0) {
	init_device();
	gdb_server_mainloop(port);
  } else {
    close(servfd);
	usleep(20000);
	start_gdb(port);
  }
}

