#include "cpu/exec.h"
#include "monitor/monitor.h"

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
  // firstly, remove the higher bits, than remove the lower bits
  return ((data << left_shift) >> left_shift) >> low;
}

uint32_t get_opcode(uint32_t instr) {
  return get_bits(instr, 31, 26);
}

uint32_t get_funct(uint32_t instr) {
  return get_bits(instr, 5, 0);
}

void decode_r_format(uint32_t instr, uint32_t *rs,
    uint32_t *rt, uint32_t *rd, uint32_t *shamt, uint32_t *funct) {
  *rs = get_bits(instr, 25, 21);
  *rt = get_bits(instr, 20, 16);
  *rd = get_bits(instr, 15, 11);
  *shamt = get_bits(instr, 10, 6);
  *funct = get_bits(instr, 5, 0);
}

exec_func gp0_table[64] = {
  /* 0x00 */    inv, inv, inv, inv,
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

void exec_gp0(vaddr_t *pc, uint32_t instr) {
  gp0_table[get_funct(instr)](pc, instr);
}

exec_func opcode_table[64] = {
  /* 0x00 */    exec_gp0, inv, inv, inv,
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

void exec_wrapper(bool print_flag) {
  uint32_t instr = instr_fetch(&cpu.pc, 4);
  opcode_table[get_opcode(instr)](&cpu.pc, instr);
}
