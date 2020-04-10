#include <signal.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "cpu.h"
#include "memory.h"
#include "qemu.h"

void cpu_exec(uint64_t);

void print_qemu_registers(qemu_regs_t *regs) {
  eprintf(
      "$pc:    0x%08x    $hi:    0x%08x    $lo:    "
      "0x%08x\n",
      regs->pc - 4, regs->hi, regs->lo);
  eprintf(
      "$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n",
      regs->gpr[0], regs->gpr[1], regs->gpr[2],
      regs->gpr[3]);
  eprintf(
      "$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n",
      regs->gpr[4], regs->gpr[5], regs->gpr[6],
      regs->gpr[7]);
  eprintf(
      "$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n",
      regs->gpr[8], regs->gpr[9], regs->gpr[10],
      regs->gpr[11]);
  eprintf(
      "$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n",
      regs->gpr[12], regs->gpr[13], regs->gpr[14],
      regs->gpr[15]);
  eprintf(
      "$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n",
      regs->gpr[16], regs->gpr[17], regs->gpr[18],
      regs->gpr[19]);
  eprintf(
      "$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n",
      regs->gpr[20], regs->gpr[21], regs->gpr[22],
      regs->gpr[23]);
  eprintf(
      "$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n",
      regs->gpr[24], regs->gpr[25], regs->gpr[26],
      regs->gpr[27]);
  eprintf(
      "$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n",
      regs->gpr[28], regs->gpr[29], regs->gpr[30],
      regs->gpr[31]);
}

static bool inst_is_branch(vaddr_t pc) {
  Inst inst = {.val = dbg_vaddr_read(pc, 4)};
  if (0x3 <= inst.op && inst.op <= 0x7) return true;
  if (0x14 <= inst.op && inst.op <= 0x17) return true;

  if (inst.op == 0x00) { // special table
    if (inst.func == 0x08 || inst.func == 0x0c) return true;
    return false;
  }

  if (inst.op == 0x01) { // regimm table
    if (0x00 <= inst.rt && inst.rt <= 0x03) return true;
    if (0x10 <= inst.rt && inst.rt <= 0x13) return true;
    return false;
  }

  return false;
}

void difftest_start_qemu(int port, int ppid) {
  // install a parent death signal in the child
  int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
  if (r == -1) { panic("prctl error"); }

  if (getppid() != ppid) { panic("parent has died"); }

  close(0); // close STDIN

  extern char *symbol_file;
  qemu_start(symbol_file, port);
}

void difftest_check_registers(qemu_regs_t *regs) {
  // diff
  CPUAssert(regs->pc == cpu.pc,
      "differ at pc:{%08x <> %08x}\n", cpu.pc, regs->pc);

  // diff general registers
  for (int i = 0; i < 32; i++) {
    CPUAssert(regs->gpr[i] == cpu.gpr[i],
        "differ at %08x, gpr[%d]:{%08x <> %08x}\n", cpu.pc,
        i, regs->gpr[i], cpu.gpr[i]);
  }

  CPUAssert(regs->hi == cpu.hi,
      "differ at %08x, hi:{%08x <> %08x}\n", cpu.pc,
      regs->hi, cpu.hi);

  CPUAssert(regs->lo == cpu.lo,
      "differ at %08x, lo:{%08x <> %08x}\n", cpu.pc,
      regs->lo, cpu.lo);
}

void difftest_body(int port) {
  qemu_regs_t regs;

  qemu_conn_t *conn = qemu_connect(port);

  extern vaddr_t elf_entry;
  qemu_break(conn, elf_entry);
  qemu_continue(conn);
  qemu_remove_breakpoint(conn, elf_entry);

  cpu.pc = elf_entry; // sync with qemu

  for (int i = 0; i < 32; i++) regs.gpr[i] = cpu.gpr[i];
  regs.pc = elf_entry;
  print_qemu_registers(&regs);
  qemu_setregs(conn, &regs);

  while (1) {
    print_qemu_registers(&regs);
    cpu_exec(1);

    if (nemu_state == NEMU_END) break;

    qemu_single_step(conn);
    qemu_getregs(conn, &regs);

    if (inst_is_branch(cpu.pc)) { continue; }

    print_qemu_registers(&regs);
    difftest_check_registers(&regs);
  }

  qemu_disconnect(conn);
}

void difftest() {
  int port = 1234;
  int ppid = getpid();

  if (fork() != 0) {
    difftest_body(port);
  } else {
    difftest_start_qemu(port, ppid);
  }
}
