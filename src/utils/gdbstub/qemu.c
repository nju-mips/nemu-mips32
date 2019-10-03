#include <stdlib.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <signal.h>

#include "protocol.h"
#include "cpu.h"
#include "memory.h"

/* only for debug, print the packets */
/*
#define CAT(a, b) CAT_IMPL(a, b)
#define CAT_IMPL(a, b) a ## b

#define gdb_send(conn, buf, size) ({ \
  printf("send:'%s'\n", (char *)buf); \
  gdb_send(conn, buf, size); })

#define gdb_recv(conn, size) ({\
  char * CAT(s, __LINE__) = (void*)gdb_recv(conn, size); \
  printf("recv:'%s'\n", (char *)CAT(s, __LINE__)); \
  (uint8_t*)CAT(s, __LINE__); \
  })
*/

void cpu_exec(uint64_t);

static struct gdb_conn *conn;

extern char *symbol_file;
extern vaddr_t elf_entry;

int start_qemu(int port) {
  char remote_s[100];
  const char *exec = "qemu-system-mipsel";
  snprintf(remote_s, sizeof(remote_s), "tcp::%d", port);

  execlp(exec, exec, "-S", "-gdb", remote_s,
		  "-kernel", symbol_file, NULL);
  return -1;
}

bool gdb_connect_qemu(int port) {
  // connect to gdbserver on localhost port 1234
  while ((conn = gdb_begin_inet("127.0.0.1", port)) == NULL) {
    usleep(1);
  }

  return true;
}

static bool qemu_memcpy_to_qemu_small(uint32_t dest, void *src, int len) {
  char *buf = malloc(len * 2 + 128);
  assert(buf != NULL);
  int p = sprintf(buf, "M0x%x,%x:", dest, len);
  int i;
  for (i = 0; i < len; i ++) {
    p += sprintf(buf + p, "%c%c", hex_encode(((uint8_t *)src)[i] >> 4), hex_encode(((uint8_t *)src)[i] & 0xf));
  }

  gdb_send(conn, (const uint8_t *)buf, strlen(buf));
  free(buf);

  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);
  bool ok = !strcmp((const char*)reply, "OK");
  free(reply);

  return ok;
}

bool qemu_memcpy_to_qemu(uint32_t dest, void *src, int len) {
  const int mtu = 1500;
  bool ok = true;
  while (len > mtu) {
    ok &= qemu_memcpy_to_qemu_small(dest, src, mtu);
    dest += mtu;
    src += mtu;
    len -= mtu;
  }
  ok &= qemu_memcpy_to_qemu_small(dest, src, len);
  return ok;
}

bool qemu_getregs(gdb_regs_t *r) {
  gdb_send(conn, (const uint8_t *)"g", 1);
  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);

  int i;
  uint8_t *p = reply;
  uint8_t c;
  for (i = 0; i < sizeof(gdb_regs_t) / sizeof(uint32_t); i ++) {
    c = p[8];
    p[8] = '\0';
    r->array[i] = gdb_decode_hex_str(p);
    p[8] = c;
    p += 8;
  }

  free(reply);

  return true;
}

bool qemu_setregs(gdb_regs_t *r) {
  int len = sizeof(gdb_regs_t);
  char *buf = malloc(len * 2 + 128);
  assert(buf != NULL);
  buf[0] = 'G';

  void *src = r;
  int p = 1;
  int i;
  for (i = 0; i < len; i ++) {
    p += sprintf(buf + p, "%c%c", hex_encode(((uint8_t *)src)[i] >> 4), hex_encode(((uint8_t *)src)[i] & 0xf));
  }

  gdb_send(conn, (const uint8_t *)buf, strlen(buf));
  free(buf);

  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);
  bool ok = !strcmp((const char*)reply, "OK");
  free(reply);

  return ok;
}

bool qemu_single_step(void) {
  char buf[] = "vCont;s:1";
  gdb_send(conn, (const uint8_t *)buf, strlen(buf));
  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);
  free(reply);
  return true;
}

void qemu_exit(void) {
  gdb_end(conn);
}

void qemu_break(uint32_t entry) {
  char buf[32];
  snprintf(buf, sizeof(buf), "Z0,%08x,4", entry);
  gdb_send(conn, (const uint8_t *)buf, strlen(buf));

  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);
  free(reply);
}

void qemu_remove_breakpoint(uint32_t entry) {
  char buf[32];
  snprintf(buf, sizeof(buf), "z0,%08x,4", entry);
  gdb_send(conn, (const uint8_t *)buf, strlen(buf));

  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);
  free(reply);
}

void qemu_continue() {
  char buf[] = "vCont;c:1";
  gdb_send(conn, (const uint8_t *)buf, strlen(buf));
  size_t size;
  uint8_t *reply = gdb_recv(conn, &size);
  free(reply);
}

void print_qemu_registers(gdb_regs_t *regs) {
  eprintf("$pc:    0x%08x    $hi:    0x%08x    $lo:    0x%08x\n", regs->pc - 4, regs->hi, regs->lo);
  eprintf("$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n", regs->gpr[0], regs->gpr[1], regs->gpr[2], regs->gpr[3]);
  eprintf("$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n", regs->gpr[4], regs->gpr[5], regs->gpr[6], regs->gpr[7]);
  eprintf("$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n", regs->gpr[8], regs->gpr[9], regs->gpr[10], regs->gpr[11]);
  eprintf("$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n", regs->gpr[12], regs->gpr[13], regs->gpr[14], regs->gpr[15]);
  eprintf("$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n", regs->gpr[16], regs->gpr[17], regs->gpr[18], regs->gpr[19]);
  eprintf("$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n", regs->gpr[20], regs->gpr[21], regs->gpr[22], regs->gpr[23]);
  eprintf("$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n", regs->gpr[24], regs->gpr[25], regs->gpr[26], regs->gpr[27]);
  eprintf("$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n", regs->gpr[28], regs->gpr[29], regs->gpr[30], regs->gpr[31]);
}

static bool is_branch_inst(vaddr_t pc) {
  Inst inst = { .val = dbg_vaddr_read(pc, 4) };
  if(0x3 <= inst.op && inst.op <= 0x7) return true;
  if(0x14 <= inst.op && inst.op <= 0x17) return true;

  if(inst.op == 0x00) { // special table
	if(inst.func == 0x08 || inst.func == 0x0c)
	  return true;
	return false;
  }

  if(inst.op == 0x01) { // regimm table
	if(0x00 <= inst.rt && inst.rt <= 0x03) return true;
	if(0x10 <= inst.rt && inst.rt <= 0x13) return true;
	return false;
  }

  return false;
}

void qemu_diff() {
  int port = 1234;
  int ppid_before_fork = getpid();

  if(fork() != 0) {
	gdb_regs_t regs;

    gdb_connect_qemu(port);

	qemu_break(elf_entry);
	qemu_continue();
	qemu_remove_breakpoint(elf_entry);

	cpu.pc = elf_entry; // sync with qemu

	for(int i = 0; i < 32; i++) regs.gpr[i] = cpu.gpr[i];
	regs.pc = elf_entry;
	qemu_setregs(&regs);

	while(1) {
	  cpu_exec(1);

	  if(nemu_state == NEMU_END) break;

	  qemu_single_step();
	  qemu_getregs(&regs);

	  if(is_branch_inst(cpu.pc)) {
		continue;
	  }

	  // diff
	  CPUAssert(regs.pc == cpu.pc, "differ at pc:{%08x <> %08x}\n", cpu.pc, regs.pc);

	  // diff general registers
	  for(int i = 0; i < 32; i++) {
		CPUAssert(regs.gpr[i] == cpu.gpr[i], "differ at %08x, gpr[%d]:{%08x <> %08x}\n", cpu.pc, i, regs.gpr[i], cpu.gpr[i]);
	  }

	  CPUAssert(regs.hi == cpu.hi, "differ at %08x, hi:{%08x <> %08x}\n", cpu.pc, regs.hi, cpu.hi);

	  CPUAssert(regs.lo == cpu.lo, "differ at %08x, lo:{%08x <> %08x}\n", cpu.pc, regs.lo, cpu.lo);
	}
  } else {
    // install a parent death signal in the child
    int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (r == -1) { panic("prctl error"); }

    if (getppid() != ppid_before_fork) {
      panic("parent has died");
    }

    close(0); // close STDIN

	start_qemu(port);
  }
}
