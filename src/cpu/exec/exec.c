#include "cpu/exec.h"
#include "monitor/monitor.h"

char asm_buf[80];
char *asm_buf_p;

void inv(vaddr_t *pc, Inst inst) {
  // the pc corresponding to this inst
  // pc has been updated by instr_fetch
  uint32_t ori_pc = *pc - 4;
  uint8_t *p = (uint8_t *)&inst;
  printf("invalid opcode(pc = 0x%08x): %02x %02x %02x %02x ...\n",
      ori_pc, p[0], p[1], p[2], p[3]);
  nemu_state = NEMU_END;
}

typedef void (*exec_func) (vaddr_t *, Inst);


void jr(vaddr_t *pc, Inst inst) {
	assert(inst.rt == 0 && inst.rd == 0);
	cpu.pc = cpu.gpr[inst.rs];
	sprintf(asm_buf_p, "jr %s", regs[inst.rs]);
}

#define R_SIMPLE(name, op, t)                                   \
void name(vaddr_t *pc, Inst inst) {                            \
	assert(inst.shamt == 0);                                   \
	cpu.gpr[inst.rd] = (t)cpu.gpr[inst.rs] op                 \
		(t)cpu.gpr[inst.rt];                                   \
	sprintf(asm_buf_p, "%s %s,%s,%s", #name, regs[inst.rd],    \
			regs[inst.rs], regs[inst.rt]);                    \
}

R_SIMPLE(or,   |, uint32_t)
R_SIMPLE(xor,  ^, uint32_t)
R_SIMPLE(and,  &, uint32_t)
R_SIMPLE(addu, +, uint32_t)
R_SIMPLE(subu, -, uint32_t)
R_SIMPLE(mul,  *, uint32_t)
R_SIMPLE(slt,  <, int32_t)
R_SIMPLE(sltu, <, uint32_t)

void nor(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	cpu.gpr[inst.rd] = ~(cpu.gpr[inst.rs] | cpu.gpr[inst.rt]);
	sprintf(asm_buf_p, "nor %s,%s,%s", regs[inst.rd],
			regs[inst.rs], regs[inst.rt]);
}

#undef R_SIMPLE


void mult(vaddr_t *pc, Inst inst) {
	assert(inst.rd == 0 && inst.shamt == 0);
	int64_t prod = (int64_t)(int32_t)cpu.gpr[inst.rs] * (int64_t)(int32_t)cpu.gpr[inst.rt];
	cpu.lo = (uint32_t)prod;
	cpu.hi = (uint32_t)(prod >> 32);
	sprintf(asm_buf_p, "mult %s,%s", regs[inst.rs], regs[inst.rt]);
}

void div(vaddr_t *pc, Inst inst) {
	assert(inst.rd == 0 && inst.shamt == 0);
	cpu.lo = (int32_t)cpu.gpr[inst.rs] / (int32_t)cpu.gpr[inst.rt];
	cpu.hi = (int32_t)cpu.gpr[inst.rs] % (int32_t)cpu.gpr[inst.rt];
	sprintf(asm_buf_p, "div %s,%s", regs[inst.rs], regs[inst.rt]);
}

void divu(vaddr_t *pc, Inst inst) {
	assert(inst.rd == 0 && inst.shamt == 0);
	cpu.lo = cpu.gpr[inst.rs] / cpu.gpr[inst.rt];
	cpu.hi = cpu.gpr[inst.rs] % cpu.gpr[inst.rt];
	sprintf(asm_buf_p, "divu %s,%s", regs[inst.rs], regs[inst.rt]);
}


void sll(vaddr_t *pc, Inst inst) {
	assert(inst.rs == 0);
	cpu.gpr[inst.rd] = cpu.gpr[inst.rt] << inst.shamt;
	sprintf(asm_buf_p, "sll %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
}

void sllv(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	cpu.gpr[inst.rd] = cpu.gpr[inst.rt] << (cpu.gpr[inst.rs] & 0x1f);
	sprintf(asm_buf_p, "sllv %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
}

void sra(vaddr_t *pc, Inst inst) {
	assert(inst.rs == 0);
	cpu.gpr[inst.rd] = (int32_t)cpu.gpr[inst.rt] >> inst.shamt;
	sprintf(asm_buf_p, "sra %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
}

void srav(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	cpu.gpr[inst.rd] = (int32_t)cpu.gpr[inst.rt] >> (cpu.gpr[inst.rs] & 0x1f);
	sprintf(asm_buf_p, "srav %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
}

void srl(vaddr_t *pc, Inst inst) {
	assert(inst.rs == 0);
	cpu.gpr[inst.rd] = cpu.gpr[inst.rt] >> inst.shamt;
	sprintf(asm_buf_p, "srl %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
}

void srlv(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	cpu.gpr[inst.rd] = cpu.gpr[inst.rt] >> (cpu.gpr[inst.rs] & 0x1f);
	sprintf(asm_buf_p, "srlv %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
}

void movn(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	if (cpu.gpr[inst.rt] != 0)
		cpu.gpr[inst.rd] = cpu.gpr[inst.rs];
	sprintf(asm_buf_p, "movn %s,%s,%s", regs[inst.rd], regs[inst.rs], regs[inst.rt]);
}

void movz(vaddr_t *pc, Inst inst) {
	assert(inst.shamt == 0);
	if (cpu.gpr[inst.rt] == 0)
		cpu.gpr[inst.rd] = cpu.gpr[inst.rs];
	sprintf(asm_buf_p, "movz %s,%s,%s", regs[inst.rd], regs[inst.rs], regs[inst.rt]);
}

void mfhi(vaddr_t *pc, Inst inst) {
	assert(inst.rs == 0 && inst.rt == 0 && inst.shamt == 0);
	cpu.gpr[inst.rd] = cpu.hi;
	sprintf(asm_buf_p, "mfhi %s", regs[inst.rd]);
}

void mflo(vaddr_t *pc, Inst inst) {
	assert(inst.rs == 0 && inst.rt == 0 && inst.shamt == 0);
	cpu.gpr[inst.rd] = cpu.lo;
	sprintf(asm_buf_p, "mflo %s", regs[inst.rd]);
}

void jalr(vaddr_t *pc, Inst inst) {
	assert(inst.rt == 0 && inst.shamt == 0);
	cpu.gpr[inst.rd] = *pc + 4;
	*pc = cpu.gpr[inst.rs];
	sprintf(asm_buf_p, "jalr %s,%s", regs[inst.rd], regs[inst.rs]);
}


void lui(vaddr_t *pc, Inst inst) {
	assert(inst.rs == 0);
	cpu.gpr[inst.rt] = inst.imm << 16;
	sprintf(asm_buf_p, "lui %s, 0x%x", regs[inst.rt], inst.imm);
}

void addiu(vaddr_t *pc, Inst inst) {
	int32_t offset = inst.offset;
	cpu.gpr[inst.rt] = cpu.gpr[inst.rs] + offset;
	sprintf(asm_buf_p, "addiu %s, %s, %d", regs[inst.rt], regs[inst.rs], offset);
}

void andi(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = cpu.gpr[inst.rs] & inst.imm;
	sprintf(asm_buf_p, "andi %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.imm);
}

void ori(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = cpu.gpr[inst.rs] | inst.imm;
	sprintf(asm_buf_p, "ori %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.imm);
}

void xori(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = cpu.gpr[inst.rs] ^ inst.imm;
	sprintf(asm_buf_p, "xori %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.imm);
}

void sltiu(vaddr_t *pc, Inst inst) {
	int32_t offset = inst.offset;
	cpu.gpr[inst.rt] = cpu.gpr[inst.rs] < (uint32_t)offset;
	sprintf(asm_buf_p, "sltiu %s, %s, %d", regs[inst.rt], regs[inst.rs], offset);
}

void slti(vaddr_t *pc, Inst inst) {
	int32_t offset = inst.offset;
	cpu.gpr[inst.rt] = (int32_t)cpu.gpr[inst.rs] < offset;
	sprintf(asm_buf_p, "slti %s, %s, %d", regs[inst.rt], regs[inst.rs], offset);
}

void swl(vaddr_t *pc, Inst inst) {
    uint32_t waddr = cpu.gpr[inst.rs] + inst.offset;
	int idx = waddr & 0x3;
    int len = idx + 1;
    uint32_t wdata = cpu.gpr[inst.rt] >> ((4 - idx) * 8);

    vaddr_write((waddr >> 2) << 2, len, wdata);
    sprintf(asm_buf_p, "swl %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void swr(vaddr_t *pc, Inst inst) {
    uint32_t waddr = cpu.gpr[inst.rs] + inst.offset;
    int len = 4 - (waddr & 0x3);
    uint32_t wdata = cpu.gpr[inst.rt];

    vaddr_write(waddr, len, wdata);
    sprintf(asm_buf_p, "swr %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void sw(vaddr_t *pc, Inst inst) {
	vaddr_write(cpu.gpr[inst.rs] + inst.offset, 4, cpu.gpr[inst.rt]);
	sprintf(asm_buf_p, "sw %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void sh(vaddr_t *pc, Inst inst) {
	vaddr_write(cpu.gpr[inst.rs] + inst.offset, 2, cpu.gpr[inst.rt]);
	sprintf(asm_buf_p, "sh %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void sb(vaddr_t *pc, Inst inst) {
	vaddr_write(cpu.gpr[inst.rs] + inst.offset, 1, cpu.gpr[inst.rt]);
	sprintf(asm_buf_p, "sb %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lwl(vaddr_t *pc, Inst inst) {
    uint32_t raddr = cpu.gpr[inst.rs] + inst.offset;
    int len = (raddr & 0x3) + 1;
    uint32_t rdata = vaddr_read((raddr >> 2) << 2, len);

    if (len < 4)
      cpu.gpr[inst.rt] = rdata << ((4 - len) * 8) | ((uint32_t)cpu.gpr[inst.rt] << (len * 8)) >> (len * 8);
    else
      cpu.gpr[inst.rt] = rdata;
    sprintf(asm_buf_p, "lwl %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lwr(vaddr_t *pc, Inst inst) {
    uint32_t raddr = cpu.gpr[inst.rs] + inst.offset;
    int idx = raddr & 0x3;
    int len = 4 - idx;
    uint32_t rdata = vaddr_read(raddr, len);
    if (len < 4)
      cpu.gpr[inst.rt] = (rdata << idx * 8) >> (idx * 8) | ((uint32_t)cpu.gpr[inst.rt] >> (len * 8)) << (len * 8);
    else
      cpu.gpr[inst.rt] = (rdata << idx * 8) >> (idx * 8);
    sprintf(asm_buf_p, "lwr %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lw(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.offset, 4);
	sprintf(asm_buf_p, "lw %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lb(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = (int32_t)(int8_t)vaddr_read(cpu.gpr[inst.rs] + inst.offset, 1);
	sprintf(asm_buf_p, "lb %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lbu(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.offset, 1);
	sprintf(asm_buf_p, "lbu %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lh(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = (int32_t)(int16_t)vaddr_read(cpu.gpr[inst.rs] + inst.offset, 2);
	sprintf(asm_buf_p, "lh %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void lhu(vaddr_t *pc, Inst inst) {
	cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.offset, 2);
	sprintf(asm_buf_p, "lhu %s, %d(%s)", regs[inst.rt], inst.offset, regs[inst.rs]);
}

void beq(vaddr_t *pc, Inst inst) {
	if (cpu.gpr[inst.rs] == cpu.gpr[inst.rt])
		*pc += inst.offset;
	sprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.offset);
}

void bne(vaddr_t *pc, Inst inst) {
	if (cpu.gpr[inst.rs] != cpu.gpr[inst.rt])
		*pc += inst.offset << 2;
	sprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.offset);
}

void blez(vaddr_t *pc, Inst inst) {
	assert(inst.rt == 0);
	if ((int32_t)cpu.gpr[inst.rs] <= 0)
		*pc += inst.offset << 2;
	sprintf(asm_buf_p, "blez %s,0x%x", regs[inst.rs], inst.offset);
}

void bgtz(vaddr_t *pc, Inst inst) {
	if ((int32_t)cpu.gpr[inst.rs] > 0)
		*pc += inst.offset << 2;
	sprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.offset);
}

void bltz(vaddr_t *pc, Inst inst) {
	if ((int32_t)cpu.gpr[inst.rs] < 0)
		*pc += inst.offset << 2;
	sprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.offset);
}

void jal(vaddr_t *pc, Inst inst) {
	cpu.gpr[31] = *pc + 4;
	*pc = (*pc & 0xf0000000) | (inst.addr << 2);
	sprintf(asm_buf_p, "jal %x", *pc);
}

void j(vaddr_t *pc, Inst inst) {
	*pc = (*pc & 0xf0000000) | (inst.addr << 2);
	sprintf(asm_buf_p, "j %x", *pc);
}

exec_func special_table[64] = {
  /* 0x00 */    sll, inv, srl, sra,
  /* 0x04 */	sllv, inv, srlv, srav,
  /* 0x08 */	jr, jalr, movz, movn,
  /* 0x0c */	inv, inv, inv, inv,
  /* 0x10 */	mfhi, inv, mflo, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	mult, inv, div, divu,
  /* 0x1c */	inv, inv, inv, inv,
  /* 0x20 */	inv, addu, inv, subu,
  /* 0x24 */	and, or, xor, nor,
  /* 0x28 */	inv, inv, slt, sltu,
  /* 0x2c */	inv, inv, inv, inv,
  /* 0x30 */	inv, inv, inv, inv,
  /* 0x34 */	inv, inv, inv, inv,
  /* 0x38 */	inv, inv, inv, inv,
  /* 0x3c */	inv, inv, inv, inv
};

void exec_special(vaddr_t *pc, Inst inst) {
  special_table[inst.func](pc, inst);
}

exec_func special2_table[64] = {
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

void exec_special2(vaddr_t *pc, Inst inst) {
  special2_table[inst.func](pc, inst);
}

exec_func regimm_table[64] = {
  /* 0x00 */    bltz, inv, inv, inv,
  /* 0x04 */	inv, inv, inv, inv,
  /* 0x08 */	inv, inv, inv, inv,
  /* 0x0c */	inv, inv, inv, inv,
  /* 0x10 */	inv, inv, inv, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	inv, inv, inv, inv,
  /* 0x1c */	inv, inv, inv, inv,
};

void exec_regimm(vaddr_t *pc, Inst inst) {
  regimm_table[inst.rt](pc, inst);
}

exec_func opcode_table[64] = {
  /* 0x00 */    exec_special, exec_regimm, j, jal,
  /* 0x04 */	beq, bne, blez, bgtz,
  /* 0x08 */	inv, addiu, slti, sltiu,
  /* 0x0c */	andi, ori, xori, lui,
  /* 0x10 */	inv, inv, inv, inv,
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
  /* 0x3c */	inv, inv, inv, inv
};

void exec_wrapper(bool print_flag) {
	asm_buf_p = asm_buf;
	asm_buf_p += sprintf(asm_buf_p, "%8x:    ", cpu.pc);
	
	Inst inst = { .val = instr_fetch(&cpu.pc, 4) };

	asm_buf_p += sprintf(asm_buf_p, "%08x    ", inst.val);

	opcode_table[inst.op](&cpu.pc, inst);

	if (print_flag)
		puts(asm_buf);
	Log_write("%s\n", asm_buf);
}
