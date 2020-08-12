#include <signal.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>

#include "cpu/memory.h"
#include "cpu/reg.h"
#include "utils/qemu.h"

void cpu_exec(uint64_t);

void print_qemu_registers(qemu_regs_t *regs) {
  eprintf("$pc:%08x\n", regs->pc - 4);
  eprintf(
      "$cs:0x%08x  $hi:0x%08x  $lo:0x%08x  $sr:0x%08x\n",
      regs->cause, regs->hi, regs->lo, regs->sr);
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

bool inst_is_branch(Inst inst) {
  if (0x2 <= inst.op && inst.op <= 0x7) return true;
  if (0x14 <= inst.op && inst.op <= 0x17) return true;

  if (inst.op == 0x00) { // special table
    if (inst.func == 0x08 || inst.func == 0x9) return true;
    return false;
  }

  if (inst.op == 0x01) { // regimm table
    if (0x00 <= inst.rt && inst.rt <= 0x03) return true;
    if (0x10 <= inst.rt && inst.rt <= 0x13) return true;
    return false;
  }

  return false;
}

bool inst_branch_taken(Inst inst) {
  int rs = cpu.gpr[inst.rs];
  int rt = cpu.gpr[inst.rt];
  switch (inst.op) {
  case 0x01: /* regimm I-type */
    switch (inst.rt) {
    case 0x00: /* bltz */
    case 0x10: /* bltzal */ return rs < 0;
    case 0x01: /* bgez */
    case 0x11: /* bgezal */ return rs >= 0;
    case 0x02: /* bltzl */
    case 0x12: /* bltzall */ return rs < 0;
    case 0x03: /* bgezl */
    case 0x13: /* bgezall */ return rs >= 0;
    }
  case 0x04: /* beq */
  case 0x14: /* beql */ return rs == rt;
  case 0x05: /* bne */
  case 0x15: /* bnel */ return rs != rt;
  case 0x06: /* blez */
  case 0x16: /* blezl */ return rs <= 0;
  case 0x07: /* bgtz */
  case 0x17: /* bgtzl */ return rs > 0;
  }
  return inst_is_branch(inst);
}

bool inst_is_mfc0(Inst inst) {
  if (inst.op == 0x10 && inst.rs == 0x0 && inst.shamt == 0)
    return true;
  return false;
}

bool inst_is_load_mmio(Inst inst, qemu_regs_t *regs) {
  if (inst.op == 0x23) {
    uint32_t addr = regs->gpr[inst.rs] + inst.simm;
    return 0xA0000000 <= addr && addr < 0xC0000000;
  }
  return false;
}

bool inst_is_store_mmio(Inst inst, qemu_regs_t *regs) {
  if (inst.op == 0x2b) {
    uint32_t addr = regs->gpr[inst.rs] + inst.simm;
    return 0xA0000000 <= addr && addr < 0xC0000000;
  }
  return false;
}

bool inst_is_mmio(Inst inst, qemu_regs_t *regs) {
  return inst_is_load_mmio(inst, regs) ||
         inst_is_store_mmio(inst, regs);
}

bool pc_in_ex_entry(uint32_t pc) {
  return pc == 0xbfc00200 || pc == 0xbfc00380 ||
         pc == 0xbfc00400 || pc == 0x80000000 ||
         pc == 0x80000180 || pc == 0x80000200;
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

#define DiffAssert(cond, fmt, ...)                        \
  do {                                                    \
    if (!(cond)) {                                        \
      nemu_epilogue();                                    \
      eprintf("nemu: %s:%d: %s: Assertion `%s' failed\n", \
          __FILE__, __LINE__, __func__, #cond);           \
      eprintf("\e[1;31mmessage: " fmt "\e[0m\n",          \
          ##__VA_ARGS__);                                 \
      difftest_finish_qemu(conn);                         \
    }                                                     \
  } while (0)

void __attribute__((noinline))
difftest_finish_qemu(qemu_conn_t *conn) {
  for (int i = 0; i < 2; i++) {
    qemu_regs_t regs = {0};
    qemu_single_step(conn);
    qemu_getregs(conn, &regs);
    print_qemu_registers(&regs);
  }
  abort();
}

void difftest_check_registers(
    qemu_regs_t *regs, qemu_conn_t *conn) {
  DiffAssert(regs->pc == cpu.pc,
      "differ at pc:{%08x <> %08x}\n", regs->pc, cpu.pc);

  for (int i = 0; i < 32; i++) {
    DiffAssert(regs->gpr[i] == cpu.gpr[i],
        "differ at %08x, gpr[%d]:{%08x <> %08x}\n", cpu.pc,
        i, regs->gpr[i], cpu.gpr[i]);
  }

  DiffAssert(regs->hi == cpu.hi,
      "differ at %08x, hi:{%08x <> %08x}\n", cpu.pc,
      regs->hi, cpu.hi);

  DiffAssert(regs->lo == cpu.lo,
      "differ at %08x, lo:{%08x <> %08x}\n", cpu.pc,
      regs->lo, cpu.lo);

  DiffAssert(regs->sr == cpu.cp0.cpr[CP0_STATUS][0],
      "differ at %08x, status:{%08x <> %08x}\n", cpu.pc,
      regs->sr, cpu.cp0.cpr[CP0_STATUS][0]);
  DiffAssert(regs->cause == cpu.cp0.cpr[CP0_CAUSE][0],
      "differ at %08x, cause:{%08x <> %08x}\n", cpu.pc,
      regs->cause, cpu.cp0.cpr[CP0_CAUSE][0]);
}

void difftest_sync_regs(
    qemu_conn_t *conn, Inst inst, qemu_regs_t *regs) {
  if (inst_is_mfc0(inst)) {
    if (inst.rd == CP0_PRID || inst.rd == CP0_CONFIG) {
      regs->gpr[inst.rt] = cpu.gpr[inst.rt];
      qemu_setregs(conn, regs);
    } else if (inst.rd == CP0_COUNT) {
      cpu.gpr[inst.rt] = regs->gpr[inst.rt];
    } else if (inst.rd == CP0_CAUSE) {
      cp0_cause_t *qemu_cause = (cp0_cause_t *)&regs->cause;
      if ((qemu_cause->ExcCode == EXC_IBE ||
              qemu_cause->ExcCode == EXC_DBE) &&
          (cpu.cp0.cause.ExcCode == EXC_AdEL ||
              cpu.cp0.cause.ExcCode == EXC_AdES)) {
        qemu_cause->ExcCode = cpu.cp0.cause.ExcCode;
        regs->gpr[inst.rt] = cpu.gpr[inst.rt];
        qemu_setregs(conn, regs);
      }
    }
  } else if (inst_is_load_mmio(inst, regs)) {
    regs->pc = cpu.pc;
    regs->gpr[inst.rt] = cpu.gpr[inst.rt];
    qemu_setregs(conn, regs);
  } else if (inst_is_store_mmio(inst, regs)) {
    regs->pc = cpu.pc;
    qemu_setregs(conn, regs);
  }
}

void difftest_body(int port) {
  qemu_regs_t regs = {0};

  qemu_conn_t *conn = qemu_connect(port);

  extern vaddr_t elf_entry;
  qemu_break(conn, elf_entry);
  qemu_continue(conn);
  qemu_remove_breakpoint(conn, elf_entry);

  cpu.pc = elf_entry; // sync with qemu

  qemu_getregs(conn, &regs);

  for (int i = 0; i < 32; i++) regs.gpr[i] = cpu.gpr[i];
  regs.pc = elf_entry;
  qemu_setregs(conn, &regs);

  while (1) {
    Inst inst = {.val = dbg_vaddr_read(cpu.pc, 4)};
    cpu_exec(1);
    printf("NEMU: %08x %d %d\n", cpu.pc,
        inst_is_branch(inst), pc_in_ex_entry(cpu.pc));

    if (nemu_state == NEMU_END) break;

    if (!inst_is_mmio(inst, &regs)) qemu_single_step(conn);
    qemu_getregs(conn, &regs);
    cp0_cause_t *qemu_cause = (cp0_cause_t *)&regs.cause;
    if (cpu.pc != regs.pc &&
        (inst_is_branch(inst) || pc_in_ex_entry(cpu.pc))) {
      difftest_sync_regs(conn, inst, &regs);
      inst.val = dbg_vaddr_read(cpu.pc, 4);
      cpu_exec(1);

      printf("NEMU: %08x -\n", cpu.pc);
    }
    difftest_sync_regs(conn, inst, &regs);
    printf("QEMU: %08x\n", regs.pc);

    cpu.cp0.cause.IP = qemu_cause->IP;
    cpu.cp0.cause.TI = qemu_cause->TI;
    difftest_check_registers(&regs, conn);
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
