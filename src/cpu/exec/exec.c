#include "cpu/exec.h"
#include "monitor/monitor.h"

char asm_buf[80];
char *asm_buf_p;

void inv(vaddr_t *pc, uint32_t instr) {
  // the pc corresponding to this instr
  // pc has been updated by instr_fetch
  uint32_t ori_pc = *pc - 4;
  uint8_t *p = (uint8_t *)&instr;
  printf("invalid opcode(pc = 0x%08x): %02x %02x %02x %02x ...\n",
      ori_pc, p[0], p[1], p[2], p[3]);
  nemu_state = NEMU_END;
}

typedef void (*exec_func) (vaddr_t *, uint32_t);

// get bits in range [high, low]
uint32_t get_bits(uint32_t data, uint32_t high, uint32_t low) {
  Assert(high >= low && high <= 31 && low >= 0, "get_bits: invalid range");
  int left_shift = 31 - high;
  // firstly, remove the higher bits, then remove the lower bits
  return ((data << left_shift) >> left_shift) >> low;
}

uint32_t get_opcode(uint32_t instr) {
  return get_bits(instr, 31, 26);
}

uint32_t get_funct(uint32_t instr) {
  return get_bits(instr, 5, 0);
}

void decode_r_format(uint32_t instr, uint32_t *rs,
    uint32_t *rt, uint32_t *rd, uint32_t *shamt) {
  *rs = get_bits(instr, 25, 21);
  *rt = get_bits(instr, 20, 16);
  *rd = get_bits(instr, 15, 11);
  *shamt = get_bits(instr, 10, 6);
  // *funct = get_bits(instr, 5, 0);
}

void jr(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, hint;
	decode_r_format(instr, &rs, &rt, &rd, &hint);
	assert(rt == 0 && rd == 0);
	cpu.pc = cpu.gpr[rs];
	sprintf(asm_buf_p, "jr %s", regs[rs]);
}

void or(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] | cpu.gpr[rt];
	sprintf(asm_buf_p, "or %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void nor(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = ~(cpu.gpr[rs] | cpu.gpr[rt]);
	sprintf(asm_buf_p, "nor %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void xor(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] ^ cpu.gpr[rt];
	sprintf(asm_buf_p, "xor %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void and(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] & cpu.gpr[rt];
	sprintf(asm_buf_p, "and %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void addu(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] + cpu.gpr[rt];
	sprintf(asm_buf_p, "addu %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void subu(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] - cpu.gpr[rt];
	sprintf(asm_buf_p, "subu %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void mul(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] * cpu.gpr[rt];
	sprintf(asm_buf_p, "mul %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void mult(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, dummy1, dummy2;
	decode_r_format(instr, &rs, &rt, &dummy1, &dummy2);
	assert(dummy1 == 0 && dummy2 == 0);
	int64_t prod = (int64_t)(int32_t)cpu.gpr[rs] * (int64_t)(int32_t)cpu.gpr[rt];
	cpu.lo = (uint32_t)prod;
	cpu.hi = (uint32_t)(prod >> 32);
	sprintf(asm_buf_p, "mult %s,%s", regs[rs], regs[rt]);
}

void div(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, dummy1, dummy2;
	decode_r_format(instr, &rs, &rt, &dummy1, &dummy2);
	assert(dummy1 == 0 && dummy2 == 0);
	cpu.lo = (int32_t)cpu.gpr[rs] / (int32_t)cpu.gpr[rt];
	cpu.hi = (int32_t)cpu.gpr[rs] % (int32_t)cpu.gpr[rt];
	sprintf(asm_buf_p, "div %s,%s", regs[rs], regs[rt]);
}

void sltu(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rs] < cpu.gpr[rt];
	sprintf(asm_buf_p, "sltu %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void slt(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = (int32_t)cpu.gpr[rs] < (int32_t)cpu.gpr[rt];
	sprintf(asm_buf_p, "slt %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void sll(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, sa;
	decode_r_format(instr, &rs, &rt, &rd, &sa);
	assert(rs == 0);
	cpu.gpr[rd] = cpu.gpr[rt] << sa;
	sprintf(asm_buf_p, "sll %s,%s,0x%x", regs[rd], regs[rt], sa);
}

void sllv(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rt] << (cpu.gpr[rs] & 0x1f);
	sprintf(asm_buf_p, "sllv %s,%s,%s", regs[rd], regs[rt], regs[rs]);
}

void sra(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, sa;
	decode_r_format(instr, &rs, &rt, &rd, &sa);
	assert(rs == 0);
	cpu.gpr[rd] = (int32_t)cpu.gpr[rt] >> sa;
	sprintf(asm_buf_p, "sra %s,%s,0x%x", regs[rd], regs[rt], sa);
}

void srav(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = (int32_t)cpu.gpr[rt] >> (cpu.gpr[rs] & 0x1f);
	sprintf(asm_buf_p, "srav %s,%s,%s", regs[rd], regs[rt], regs[rs]);
}

void srl(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, sa;
	decode_r_format(instr, &rs, &rt, &rd, &sa);
	assert(rs == 0);
	cpu.gpr[rd] = cpu.gpr[rt] >> sa;
	sprintf(asm_buf_p, "srl %s,%s,0x%x", regs[rd], regs[rt], sa);
}

void srlv(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	cpu.gpr[rd] = cpu.gpr[rt] >> (cpu.gpr[rs] & 0x1f);
	sprintf(asm_buf_p, "srlv %s,%s,%s", regs[rd], regs[rt], regs[rs]);
}

void movn(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	if (cpu.gpr[rt] != 0)
		cpu.gpr[rd] = cpu.gpr[rs];
	sprintf(asm_buf_p, "movn %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void movz(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(dummy == 0);
	if (cpu.gpr[rt] == 0)
		cpu.gpr[rd] = cpu.gpr[rs];
	sprintf(asm_buf_p, "movz %s,%s,%s", regs[rd], regs[rs], regs[rt]);
}

void mfhi(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(rs == 0 && rt == 0 && dummy == 0);
	cpu.gpr[rd] = cpu.hi;
	sprintf(asm_buf_p, "mfhi %s", regs[rd]);
}

void mflo(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, dummy;
	decode_r_format(instr, &rs, &rt, &rd, &dummy);
	assert(rs == 0 && rt == 0 && dummy == 0);
	cpu.gpr[rd] = cpu.lo;
	sprintf(asm_buf_p, "mflo %s", regs[rd]);
}

void jalr(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, rd, hint;
	decode_r_format(instr, &rs, &rt, &rd, &hint);
	assert(rt == 0 && hint == 0);
	cpu.gpr[rd] = *pc + 4;
	*pc = cpu.gpr[rs];
	sprintf(asm_buf_p, "jalr %s,%s", regs[rd], regs[rs]);
}

void decode_i_format(uint32_t instr, uint32_t *rs,
    uint32_t *rt, uint32_t *imm) {
  *rs = get_bits(instr, 25, 21);
  *rt = get_bits(instr, 20, 16);
  *imm = get_bits(instr, 15, 0);
}

void lui(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	assert(rs == 0);
	cpu.gpr[rt] = imm << 16;
	sprintf(asm_buf_p, "lui %s, 0x%x", regs[rt], imm);
}

void addiu(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	int32_t simm = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = cpu.gpr[rs] + simm;
	sprintf(asm_buf_p, "addiu %s, %s, %d", regs[rt], regs[rs], simm);
}

void andi(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	cpu.gpr[rt] = cpu.gpr[rs] & imm;
	sprintf(asm_buf_p, "andi %s, %s, 0x%x", regs[rt], regs[rs], imm);
}

void ori(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	cpu.gpr[rt] = cpu.gpr[rs] | imm;
	sprintf(asm_buf_p, "ori %s, %s, 0x%x", regs[rt], regs[rs], imm);
}

void xori(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	cpu.gpr[rt] = cpu.gpr[rs] ^ imm;
	sprintf(asm_buf_p, "xori %s, %s, 0x%x", regs[rt], regs[rs], imm);
}

void sltiu(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	int32_t simm = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = cpu.gpr[rs] < (uint32_t)simm;
	sprintf(asm_buf_p, "sltiu %s, %s, %d", regs[rt], regs[rs], simm);
}

void slti(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	int32_t simm = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = (int32_t)cpu.gpr[rs] < simm;
	sprintf(asm_buf_p, "slti %s, %s, %d", regs[rt], regs[rs], simm);
}

void sw(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	vaddr_write(cpu.gpr[base] + offset, 4, cpu.gpr[rt]);
	sprintf(asm_buf_p, "sw %s, %d(%s)", regs[rt], offset, regs[base]);
}

void sh(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	vaddr_write(cpu.gpr[base] + offset, 2, cpu.gpr[rt]);
	sprintf(asm_buf_p, "sh %s, %d(%s)", regs[rt], offset, regs[base]);
}

void sb(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	vaddr_write(cpu.gpr[base] + offset, 1, cpu.gpr[rt]);
	sprintf(asm_buf_p, "sb %s, %d(%s)", regs[rt], offset, regs[base]);
}

void lw(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = vaddr_read(cpu.gpr[base] + offset, 4);
	sprintf(asm_buf_p, "lw %s, %d(%s)", regs[rt], offset, regs[base]);
}

void lb(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = (int32_t)(int8_t)vaddr_read(cpu.gpr[base] + offset, 1);
	sprintf(asm_buf_p, "lb %s, %d(%s)", regs[rt], offset, regs[base]);
}

void lbu(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = vaddr_read(cpu.gpr[base] + offset, 1);
	sprintf(asm_buf_p, "lbu %s, %d(%s)", regs[rt], offset, regs[base]);
}

void lh(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = (int32_t)(int16_t)vaddr_read(cpu.gpr[base] + offset, 2);
	sprintf(asm_buf_p, "lh %s, %d(%s)", regs[rt], offset, regs[base]);
}

void lhu(vaddr_t *pc, uint32_t instr) {
	uint32_t base, rt, imm;
	decode_i_format(instr, &base, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm;
	cpu.gpr[rt] = vaddr_read(cpu.gpr[base] + offset, 2);
	sprintf(asm_buf_p, "lhu %s, %d(%s)", regs[rt], offset, regs[base]);
}

void beq(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm << 2;
	if (cpu.gpr[rs] == cpu.gpr[rt])
		*pc += offset;
	sprintf(asm_buf_p, "beq %s,%s,0x%x", regs[rs], regs[rt], offset);
}

void bne(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	int32_t offset = (int32_t)(int16_t)imm << 2;
	if (cpu.gpr[rs] != cpu.gpr[rt])
		*pc += offset;
	sprintf(asm_buf_p, "beq %s,%s,0x%x", regs[rs], regs[rt], offset);
}

void blez(vaddr_t *pc, uint32_t instr) {
	uint32_t rs, rt, imm;
	decode_i_format(instr, &rs, &rt, &imm);
	assert(rt == 0);
	int32_t offset = (int32_t)(int16_t)imm << 2;
	if ((int32_t)cpu.gpr[rs] <= 0)
		*pc += offset;
	sprintf(asm_buf_p, "blez %s,0x%x", regs[rs], offset);
}

void decode_j_format(uint32_t instr, uint32_t *addr) {
  *addr = get_bits(instr, 25, 0);
}

void jal(vaddr_t *pc, uint32_t instr) {
	uint32_t instr_index;
	decode_j_format(instr, &instr_index);
	cpu.gpr[31] = *pc + 4;
	*pc = (*pc & 0xf0000000) | (instr_index << 2);
	sprintf(asm_buf_p, "jal %x", *pc);
}

void j(vaddr_t *pc, uint32_t instr) {
	uint32_t instr_index;
	decode_j_format(instr, &instr_index);
	*pc = (*pc & 0xf0000000) | (instr_index << 2);
	sprintf(asm_buf_p, "j %x", *pc);
}

exec_func special_table[64] = {
  /* 0x00 */    sll, inv, srl, sra,
  /* 0x04 */	sllv, inv, srlv, srav,
  /* 0x08 */	jr, jalr, movz, movn,
  /* 0x0c */	inv, inv, inv, inv,
  /* 0x10 */	mfhi, inv, mflo, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	mult, inv, div, inv,
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

void exec_special(vaddr_t *pc, uint32_t instr) {
  special_table[get_funct(instr)](pc, instr);
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

void exec_special2(vaddr_t *pc, uint32_t instr) {
  special2_table[get_funct(instr)](pc, instr);
}

exec_func opcode_table[64] = {
  /* 0x00 */    exec_special, inv, j, jal,
  /* 0x04 */	beq, bne, blez, inv,
  /* 0x08 */	inv, addiu, slti, sltiu,
  /* 0x0c */	andi, ori, xori, lui,
  /* 0x10 */	inv, inv, inv, inv,
  /* 0x14 */	inv, inv, inv, inv,
  /* 0x18 */	inv, inv, inv, inv,
  /* 0x1c */	exec_special2, inv, inv, inv,
  /* 0x20 */	lb, lh, inv, lw,
  /* 0x24 */	lbu, lhu, inv, inv,
  /* 0x28 */	sb, sh, inv, sw,
  /* 0x2c */	inv, inv, inv, inv,
  /* 0x30 */	inv, inv, inv, inv,
  /* 0x34 */	inv, inv, inv, inv,
  /* 0x38 */	inv, inv, inv, inv,
  /* 0x3c */	inv, inv, inv, inv
};

void exec_wrapper(bool print_flag) {
	asm_buf_p = asm_buf;
	asm_buf_p += sprintf(asm_buf_p, "%8x:    ", cpu.pc);

	uint32_t instr = instr_fetch(&cpu.pc, 4);

	asm_buf_p += sprintf(asm_buf_p, "%08x    ", instr);

	opcode_table[get_opcode(instr)](&cpu.pc, instr);

	if (print_flag)
		puts(asm_buf);
	Log_write("%s\n", asm_buf);
}
