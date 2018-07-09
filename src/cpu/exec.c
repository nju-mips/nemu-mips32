#include "cpu/exec.h"
#include "monitor/monitor.h"

#include <sys/stat.h>
#include <stdarg.h>

#define EXCEPTION_VECTOR_LOCATION 0x10000020

// #define dsprintf sprintf

void print_registers();

extern int nemu_state;
extern int print_commit_log;

char asm_buf[80];
char *asm_buf_p;

int dsprintf(char *buf, const char *fmt, ...) {
  if(!print_commit_log) return 0;
#if 0
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  printf("\n");
#endif
  return 0;
}

static void update_cp0_timer() {
  cpu.cp0[CP0_COUNT][0] += 5; // add 5 cycles
}

static void inv(vaddr_t *pc, Inst inst) {
  // the pc corresponding to this inst
  // pc has been updated by instr_fetch
  uint32_t ori_pc = *pc - 4;
  uint8_t *p = (uint8_t *)&inst;
  printf("invalid opcode(pc = 0x%08x): %02x %02x %02x %02x ...\n",
	  ori_pc, p[0], p[1], p[2], p[3]);
  nemu_state = NEMU_END;
}

typedef void (*exec_func) (vaddr_t *, Inst);

static inline void trigger_exception(int code) {
  cpu.cp0[CP0_EPC][0] = cpu.pc;
  cpu.pc = EXCEPTION_VECTOR_LOCATION;

  cpu.base = 0; // kernel base is zero

  cp0_status_t *status = (void *)&(cpu.cp0[CP0_STATUS][0]);
  status->EXL = 1;
  status->IE = 0;

  cp0_cause_t *cause = (void *)&(cpu.cp0[CP0_CAUSE][0]);
  cause->ExcCode = code;
}

// temporary strategy: store timer registers in C0
static void syscall(vaddr_t *pc, Inst inst) {
  trigger_exception(EXC_SYSCALL);
  dsprintf(asm_buf_p, "syscall");
}

static void breakpoint(vaddr_t *pc, Inst inst) {
  nemu_state = NEMU_STOP;
}

static void eret(vaddr_t *pc, Inst inst) {
  *pc = cpu.cp0[CP0_EPC][0];
  cp0_status_t *status = (void *)&(cpu.cp0[CP0_STATUS][0]);
  status->EXL = 0;
  status->IE = 1;
  cpu.base = cpu.cp0[CP0_BASE][0]; // resume user base
  // printf("[NEMU] NOTE: eret to user space, base at %08x\n", cpu.base);
  dsprintf(asm_buf_p, "eret");
}

static void mfc0(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = cpu.cp0[inst.rd][inst.sel];
  dsprintf(asm_buf_p, "mfc0 $%s, $%d, %d", regs[inst.rt],
	  inst.rd, inst.sel);
}

static void mtc0(vaddr_t *pc, Inst inst) {
  cpu.cp0[inst.rd][inst.sel] = cpu.gpr[inst.rt];
  if(inst.rd == CP0_SERIAL) putchar(cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "mtc0 $%s, $%d, %d", regs[inst.rt],
	  inst.rd, inst.sel);
}

static void jr(vaddr_t *pc, Inst inst) {
  assert(inst.rt == 0 && inst.rd == 0);
  *pc = cpu.gpr[inst.rs];
  dsprintf(asm_buf_p, "jr %s", regs[inst.rs]);
}

#define R_SIMPLE(name, op, t)                              \
  static void name(vaddr_t *pc, Inst inst) {                        \
	assert(inst.shamt == 0);                                 \
	cpu.gpr[inst.rd] = (t)cpu.gpr[inst.rs] op                \
	(t)cpu.gpr[inst.rt];                                     \
	dsprintf(asm_buf_p, "%s %s,%s,%s", #name, regs[inst.rd],  \
		regs[inst.rs], regs[inst.rt]);                       \
  }

  R_SIMPLE(or,   |, uint32_t)
  R_SIMPLE(xor,  ^, uint32_t)
  R_SIMPLE(and,  &, uint32_t)
  R_SIMPLE(add,  +, int32_t)
  R_SIMPLE(addu, +, uint32_t)
  R_SIMPLE(sub,  -, int32_t)
  R_SIMPLE(subu, -, uint32_t)
  R_SIMPLE(mul,  *, uint32_t)
  R_SIMPLE(slt,  <, int32_t)
R_SIMPLE(sltu, <, uint32_t)

  static void nor(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	cpu.gpr[inst.rd] = ~(cpu.gpr[inst.rs] | cpu.gpr[inst.rt]);
	dsprintf(asm_buf_p, "nor %s,%s,%s", regs[inst.rd],
		regs[inst.rs], regs[inst.rt]);
  }

#undef R_SIMPLE


static void mult(vaddr_t *pc, Inst inst) {
  assert(inst.rd == 0 && inst.shamt == 0);
  int64_t prod = (int64_t)(int32_t)cpu.gpr[inst.rs] * (int64_t)(int32_t)cpu.gpr[inst.rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
  dsprintf(asm_buf_p, "mult %s,%s", regs[inst.rs], regs[inst.rt]);
}

static void multu(vaddr_t *pc, Inst inst) {
  assert(inst.rd == 0 && inst.shamt == 0);
  uint64_t prod = (uint64_t)cpu.gpr[inst.rs] * (uint64_t)cpu.gpr[inst.rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
  dsprintf(asm_buf_p, "multu %s,%s", regs[inst.rs], regs[inst.rt]);
}

static void divide(vaddr_t *pc, Inst inst) {
  assert(inst.rd == 0 && inst.shamt == 0);
  cpu.lo = (int32_t)cpu.gpr[inst.rs] / (int32_t)cpu.gpr[inst.rt];
  cpu.hi = (int32_t)cpu.gpr[inst.rs] % (int32_t)cpu.gpr[inst.rt];
  dsprintf(asm_buf_p, "div %s,%s", regs[inst.rs], regs[inst.rt]);
}

static void divu(vaddr_t *pc, Inst inst) {
  assert(inst.rd == 0 && inst.shamt == 0);
  cpu.lo = cpu.gpr[inst.rs] / cpu.gpr[inst.rt];
  cpu.hi = cpu.gpr[inst.rs] % cpu.gpr[inst.rt];
  dsprintf(asm_buf_p, "divu %s,%s", regs[inst.rs], regs[inst.rt]);
}


static void sll(vaddr_t *pc, Inst inst) {
  assert(inst.rs == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] << inst.shamt;
  dsprintf(asm_buf_p, "sll %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
}

static void sllv(vaddr_t *pc, Inst inst) {
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] << (cpu.gpr[inst.rs] & 0x1f);
  dsprintf(asm_buf_p, "sllv %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
}

static void sra(vaddr_t *pc, Inst inst) {
  assert(inst.rs == 0);
  cpu.gpr[inst.rd] = (int32_t)cpu.gpr[inst.rt] >> inst.shamt;
  dsprintf(asm_buf_p, "sra %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
}

static void srav(vaddr_t *pc, Inst inst) {
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = (int32_t)cpu.gpr[inst.rt] >> (cpu.gpr[inst.rs] & 0x1f);
  dsprintf(asm_buf_p, "srav %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
}

static void srl(vaddr_t *pc, Inst inst) {
  assert(inst.rs == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] >> inst.shamt;
  dsprintf(asm_buf_p, "srl %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
}

static void srlv(vaddr_t *pc, Inst inst) {
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] >> (cpu.gpr[inst.rs] & 0x1f);
  dsprintf(asm_buf_p, "srlv %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
}

static void movn(vaddr_t *pc, Inst inst) {
  assert(inst.shamt == 0);
  if (cpu.gpr[inst.rt] != 0)
	cpu.gpr[inst.rd] = cpu.gpr[inst.rs];
  dsprintf(asm_buf_p, "movn %s,%s,%s", regs[inst.rd], regs[inst.rs], regs[inst.rt]);
}

static void movz(vaddr_t *pc, Inst inst) {
  assert(inst.shamt == 0);
  if (cpu.gpr[inst.rt] == 0)
	cpu.gpr[inst.rd] = cpu.gpr[inst.rs];
  dsprintf(asm_buf_p, "movz %s,%s,%s", regs[inst.rd], regs[inst.rs], regs[inst.rt]);
}

static void mfhi(vaddr_t *pc, Inst inst) {
  assert(inst.rs == 0 && inst.rt == 0 && inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.hi;
  dsprintf(asm_buf_p, "mfhi %s", regs[inst.rd]);
}

static void mflo(vaddr_t *pc, Inst inst) {
  assert(inst.rs == 0 && inst.rt == 0 && inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.lo;
  dsprintf(asm_buf_p, "mflo %s", regs[inst.rd]);
}

static void jalr(vaddr_t *pc, Inst inst) {
  assert(inst.rt == 0 && inst.shamt == 0);
  cpu.gpr[inst.rd] = *pc + 4;
  *pc = cpu.gpr[inst.rs];
  dsprintf(asm_buf_p, "jalr %s,%s", regs[inst.rd], regs[inst.rs]);
}


static void lui(vaddr_t *pc, Inst inst) {
  assert(inst.rs == 0);
  cpu.gpr[inst.rt] = inst.uimm << 16;
  dsprintf(asm_buf_p, "lui %s, 0x%x", regs[inst.rt], inst.uimm);
}

static void addiu(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] + inst.simm;
  dsprintf(asm_buf_p, "addiu %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
}

static void andi(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] & inst.uimm;
  dsprintf(asm_buf_p, "andi %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.uimm);
}

static void ori(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] | inst.uimm;
  dsprintf(asm_buf_p, "ori %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.uimm);
}

static void xori(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] ^ inst.uimm;
  dsprintf(asm_buf_p, "xori %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.uimm);
}

static void sltiu(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] < inst.uimm;
  dsprintf(asm_buf_p, "sltiu %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
}

static void slti(vaddr_t *pc, Inst inst) {
  cpu.gpr[inst.rt] = (int32_t)cpu.gpr[inst.rs] < inst.simm;
  dsprintf(asm_buf_p, "slti %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
}


#define CHECK_ALIGNED_ADDR(align, addr) \
  Assert(((addr) & (align - 1)) == 0, "address(0x%08x) is unaligned, pc=%08x\n", (addr), cpu.pc)

static void swl(vaddr_t *pc, Inst inst) {
  uint32_t waddr = cpu.gpr[inst.rs] + inst.simm;
  int idx = waddr & 0x3;
  int len = idx + 1;
  uint32_t wdata = cpu.gpr[inst.rt] >> ((3 - idx) * 8);

  vaddr_write((waddr >> 2) << 2, len, wdata);
  dsprintf(asm_buf_p, "swl %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void swr(vaddr_t *pc, Inst inst) {
  uint32_t waddr = cpu.gpr[inst.rs] + inst.simm;
  int len = 4 - (waddr & 0x3);
  uint32_t wdata = cpu.gpr[inst.rt];

  vaddr_write(waddr, len, wdata);
  dsprintf(asm_buf_p, "swr %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void sw(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(4, cpu.gpr[inst.rs] + inst.simm);
  vaddr_write(cpu.gpr[inst.rs] + inst.simm, 4, cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "sw %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void sh(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(2, cpu.gpr[inst.rs] + inst.simm);
  vaddr_write(cpu.gpr[inst.rs] + inst.simm, 2, cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "sh %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void sb(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(1, cpu.gpr[inst.rs] + inst.simm);
  vaddr_write(cpu.gpr[inst.rs] + inst.simm, 1, cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "sb %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lwl(vaddr_t *pc, Inst inst) {
  uint32_t raddr = cpu.gpr[inst.rs] + inst.simm;
  int len = (raddr & 0x3) + 1;
  uint32_t rdata = vaddr_read((raddr >> 2) << 2, len);

  if (len < 4)
	cpu.gpr[inst.rt] = rdata << ((4 - len) * 8) | ((uint32_t)cpu.gpr[inst.rt] << (len * 8)) >> (len * 8);
  else
	cpu.gpr[inst.rt] = rdata;
  dsprintf(asm_buf_p, "lwl %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lwr(vaddr_t *pc, Inst inst) {
  uint32_t raddr = cpu.gpr[inst.rs] + inst.simm;
  int idx = raddr & 0x3;
  int len = 4 - idx;
  uint32_t rdata = vaddr_read(raddr, len);
  if (len < 4)
	cpu.gpr[inst.rt] = (rdata << idx * 8) >> (idx * 8) | ((uint32_t)cpu.gpr[inst.rt] >> (len * 8)) << (len * 8);
  else
	cpu.gpr[inst.rt] = (rdata << idx * 8) >> (idx * 8);
  dsprintf(asm_buf_p, "lwr %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lw(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(4, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.simm, 4);
  dsprintf(asm_buf_p, "lw %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lb(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(1, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = (int32_t)(int8_t)vaddr_read(cpu.gpr[inst.rs] + inst.simm, 1);
  dsprintf(asm_buf_p, "lb %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lbu(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(1, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.simm, 1);
  dsprintf(asm_buf_p, "lbu %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lh(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(2, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = (int32_t)(int16_t)vaddr_read(cpu.gpr[inst.rs] + inst.simm, 2);
  dsprintf(asm_buf_p, "lh %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}

static void lhu(vaddr_t *pc, Inst inst) {
  CHECK_ALIGNED_ADDR(2, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.simm, 2);
  dsprintf(asm_buf_p, "lhu %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
}


static void beq(vaddr_t *pc, Inst inst) {
  if (cpu.gpr[inst.rs] == cpu.gpr[inst.rt])
	*pc += inst.simm << 2;
  dsprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.simm);
}

static void bne(vaddr_t *pc, Inst inst) {
  if (cpu.gpr[inst.rs] != cpu.gpr[inst.rt])
	*pc += inst.simm << 2;
  dsprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.simm);
}

static void blez(vaddr_t *pc, Inst inst) {
  assert(inst.rt == 0);
  if ((int32_t)cpu.gpr[inst.rs] <= 0)
	*pc += inst.simm << 2;
  dsprintf(asm_buf_p, "blez %s,0x%x", regs[inst.rs], inst.simm);
}

static void bgtz(vaddr_t *pc, Inst inst) {
  if ((int32_t)cpu.gpr[inst.rs] > 0)
	*pc += inst.simm << 2;
  dsprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.simm);
}

static void bltz(vaddr_t *pc, Inst inst) {
  if ((int32_t)cpu.gpr[inst.rs] < 0)
	*pc += inst.simm << 2;
  dsprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.simm);
}

static void bgez(vaddr_t *pc, Inst inst) {
  if ((int32_t)cpu.gpr[inst.rs] >= 0)
	*pc += inst.simm << 2;
  dsprintf(asm_buf_p, "bgez %s,0x%x", regs[inst.rs], inst.simm);
}

static void bgezal(vaddr_t *pc, Inst inst) {
  cpu.gpr[31] = *pc + 4;
  if ((int32_t)cpu.gpr[inst.rs] >= 0)
	*pc += inst.simm << 2;
}

static void bltzal(vaddr_t *pc, Inst inst) {
  cpu.gpr[31] = *pc + 4;
  if ((int32_t)cpu.gpr[inst.rs] < 0)
	*pc += inst.simm << 2;
}

static void jal(vaddr_t *pc, Inst inst) {
  cpu.gpr[31] = *pc + 4;
  *pc = (*pc & 0xf0000000) | (inst.addr << 2);
  dsprintf(asm_buf_p, "jal %x", *pc);
}

static void j(vaddr_t *pc, Inst inst) {
  *pc = (*pc & 0xf0000000) | (inst.addr << 2);
  dsprintf(asm_buf_p, "j %x", *pc);
}

static exec_func special_table[64] = {
  /* 0x00 */    sll, inv, srl, sra,
  /* 0x04 */	sllv, inv, srlv, srav,
  /* 0x08 */	jr, jalr, movz, movn,
  /* 0x0c */	syscall, breakpoint, inv, inv,
  /* 0x10 */	mfhi, inv, mflo, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	mult, multu, divide, divu,
  /* 0x1c */	inv, inv, inv, inv,
  /* 0x20 */	add, addu, sub, subu,
  /* 0x24 */	and, or, xor, nor,
  /* 0x28 */	inv, inv, slt, sltu,
  /* 0x2c */	inv, inv, inv, inv,
  /* 0x30 */	inv, inv, inv, inv,
  /* 0x34 */	inv, inv, inv, inv,
  /* 0x38 */	inv, inv, inv, inv,
  /* 0x3c */	inv, inv, inv, inv
};

static void exec_special(vaddr_t *pc, Inst inst) {
  special_table[inst.func](pc, inst);
}

static exec_func special2_table[64] = {
  /* 0x00 */    inv, inv, mul, inv,
  /* 0x04 */	inv, inv, inv, inv,
  /* 0x08 */	inv, inv, inv, inv,
  /* 0x0c */	inv, inv, inv, inv,
  /* 0x10 */	inv, inv, inv, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	inv, inv, inv, inv,
  /* 0x1c */	inv, inv, inv, inv,
  /* 0x20 */	inv, inv, inv, inv,
  /* 0x24 */	inv, inv, inv, inv,
  /* 0x28 */	inv, inv, inv, inv,
  /* 0x2c */	inv, inv, inv, inv,
  /* 0x30 */	inv, inv, inv, inv,
  /* 0x34 */	inv, inv, inv, inv,
  /* 0x38 */	inv, inv, inv, inv,
  /* 0x3c */	inv, inv, inv, inv
};

static void exec_special2(vaddr_t *pc, Inst inst) {
  special2_table[inst.func](pc, inst);
}

static exec_func regimm_table[64] = {
  /* 0x00 */    bltz, bgez, inv, inv,
  /* 0x04 */	inv, inv, inv, inv,
  /* 0x08 */	inv, inv, inv, inv,
  /* 0x0c */	inv, inv, inv, inv,
  /* 0x10 */	bltzal, bgezal, inv, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	inv, inv, inv, inv,
  /* 0x1c */	inv, inv, inv, inv,
};

static void exec_regimm(vaddr_t *pc, Inst inst) {
  regimm_table[inst.rt](pc, inst);
}

static exec_func cop0_table_rs[32] = {
  /* 0x00 */    mfc0, inv, inv, inv,
  /* 0x04 */    mtc0, inv, inv, inv,
  /* 0x08 */    inv, inv, inv, inv,
  /* 0x0c */    inv, inv, inv, inv,
  /* 0x10 */    inv, inv, inv, inv,
  /* 0x14 */    inv, inv, inv, inv,
  /* 0x18 */    inv, inv, inv, inv,
  /* 0x1c */    inv, inv, inv, inv,
};

static exec_func cop0_table_func[64] = {
  /* 0x00 */    inv, inv, inv, inv,
  /* 0x04 */    inv, inv, inv, inv,
  /* 0x08 */    inv, inv, inv, inv,
  /* 0x0c */    inv, inv, inv, inv,
  /* 0x10 */    inv, inv, inv, inv,
  /* 0x14 */    inv, inv, inv, inv,
  /* 0x18 */    eret, inv, inv, inv,
  /* 0x1c */    inv, inv, inv, inv,

  /* 0x20 */    inv, inv, inv, inv,
  /* 0x24 */    inv, inv, inv, inv,
  /* 0x28 */    inv, inv, inv, inv,
  /* 0x2c */    inv, inv, inv, inv,
  /* 0x30 */    inv, inv, inv, inv,
  /* 0x34 */    inv, inv, inv, inv,
  /* 0x38 */    inv, inv, inv, inv,
  /* 0x3c */    inv, inv, inv, inv,
};

static void exec_cop0(vaddr_t *pc, Inst inst) {
  if(inst.rs & 0x10)
	cop0_table_func[inst.func](pc, inst);
  else
	cop0_table_rs[inst.rs](pc, inst);
}

static exec_func opcode_table[64] = {
  /* 0x00 */    exec_special, exec_regimm, j, jal,
  /* 0x04 */	beq, bne, blez, bgtz,
  /* 0x08 */	inv, addiu, slti, sltiu,
  /* 0x0c */	andi, ori, xori, lui,
  /* 0x10 */	exec_cop0, inv, inv, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	inv, inv, inv, inv,
  /* 0x1c */	exec_special2, inv, inv, inv,
  /* 0x20 */	lb, lh, lwl, lw,
  /* 0x24 */	lbu, lhu, lwr, inv,
  /* 0x28 */	sb, sh, swl, sw,
  /* 0x2c */	inv, inv, swr, inv,
  /* 0x30 */	inv, inv, inv, inv,
  /* 0x34 */	inv, inv, inv, inv,
  /* 0x38 */	inv, inv, inv, inv,
  /* 0x3c */	inv, inv, inv, inv,
};

void print_registers() {
  // printf("$base:     0x%08x\n", cpu.base);
  printf("$pc:    0x%08x    $hi:    0x%08x    $lo:    0x%08x\n", cpu.pc, cpu.hi, cpu.lo);
  printf("$0 :0x%08x  $at:0x%08x  $v0:0x%08x  $v1:0x%08x\n", cpu.gpr[0], cpu.gpr[1], cpu.gpr[2], cpu.gpr[3]);
  printf("$a0:0x%08x  $a1:0x%08x  $a2:0x%08x  $a3:0x%08x\n", cpu.gpr[4], cpu.gpr[5], cpu.gpr[6], cpu.gpr[7]);
  printf("$t0:0x%08x  $t1:0x%08x  $t2:0x%08x  $t3:0x%08x\n", cpu.gpr[8], cpu.gpr[9], cpu.gpr[10], cpu.gpr[11]);
  printf("$t4:0x%08x  $t5:0x%08x  $t6:0x%08x  $t7:0x%08x\n", cpu.gpr[12], cpu.gpr[13], cpu.gpr[14], cpu.gpr[15]);
  printf("$s0:0x%08x  $s1:0x%08x  $s2:0x%08x  $s3:0x%08x\n", cpu.gpr[16], cpu.gpr[17], cpu.gpr[18], cpu.gpr[19]);
  printf("$s4:0x%08x  $s5:0x%08x  $s6:0x%08x  $s7:0x%08x\n", cpu.gpr[20], cpu.gpr[21], cpu.gpr[22], cpu.gpr[23]);
  printf("$t8:0x%08x  $t9:0x%08x  $k0:0x%08x  $k1:0x%08x\n", cpu.gpr[24], cpu.gpr[25], cpu.gpr[26], cpu.gpr[27]);
  printf("$gp:0x%08x  $sp:0x%08x  $fp:0x%08x  $ra:0x%08x\n", cpu.gpr[28], cpu.gpr[29], cpu.gpr[30], cpu.gpr[31]);
}

int init_cpu() {
  assert(sizeof(cp0_status_t) == sizeof(cpu.cp0[CP0_STATUS][0]));
  assert(sizeof(cp0_cause_t) == sizeof(cpu.cp0[CP0_CAUSE][0]));
  cpu.gpr[29] = 0x18000000;
  cpu.cp0[CP0_STATUS][0] = 0x1000FF00;
  return 0;
}

void check_interrupt() {
  if(cpu.cp0[CP0_COMPARE][0] == cpu.cp0[CP0_COUNT][0]) {
	trigger_exception(EXC_INTR);
  }
}

void exec_wrapper(bool print_flag) {
  update_cp0_timer();

  asm_buf_p = asm_buf;
  asm_buf_p += dsprintf(asm_buf_p, "%8x:    ", cpu.pc);

  Inst inst = { .val = instr_fetch(&cpu.pc, 4) };

  asm_buf_p += dsprintf(asm_buf_p, "%08x    ", inst.val);

  opcode_table[inst.op](&cpu.pc, inst);

#ifdef INTR
  check_interrupt();
#endif
}
