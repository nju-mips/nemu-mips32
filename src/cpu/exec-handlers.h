/* @{{
 *   `cpu.pc' for jmp instruction
 *   `oldpc'  for delayslot
 *   `instr'  for special table
 * @}}
 */

// checked delayslot: jal, jalr, 


#ifdef ENABLE_DELAYSLOT
#define make_entry() exec_entry:
#else
#define make_entry()
#endif

#define make_exec_handler(name) name: make_exec_wrapper
#define make_exec_wrapper(...) __VA_ARGS__; goto eoe;
#define make_eoe() eoe:;

#ifdef ENABLE_DELAYSLOT
#define exec_delayslot() \
	inst.val = instr_fetch(oldpc + 4); \
	goto exec_entry;
#else
#define exec_delayslot()
#endif

static const void * special_table[64] = {
  /* 0x00 */    &&sll, &&inv, &&srl, &&sra,
  /* 0x04 */	&&sllv, &&inv, &&srlv, &&srav,
  /* 0x08 */	&&jr, &&jalr, &&movz, &&movn,
  /* 0x0c */	&&syscall, &&breakpoint, &&inv, &&inv,
  /* 0x10 */	&&mfhi, &&inv, &&mflo, &&inv,
  /* 0x14 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x18 */	&&mult, &&multu, &&divide, &&divu,
  /* 0x1c */	&&inv, &&inv, &&inv, &&inv,
  /* 0x20 */	&&add, &&addu, &&sub, &&subu,
  /* 0x24 */	&&and, &&or, &&xor, &&nor,
  /* 0x28 */	&&inv, &&inv, &&slt, &&sltu,
  /* 0x2c */	&&inv, &&inv, &&inv, &&inv,
  /* 0x30 */	&&tge, &&tgeu, &&tlt, &&tltu,
  /* 0x34 */	&&teq, &&inv, &&tne, &&inv,
  /* 0x38 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x3c */	&&inv, &&inv, &&inv, &&inv,
};


static const void * special2_table[64] = {
  /* 0x00 */    &&inv, &&inv, &&mul, &&inv,
  /* 0x04 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x08 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x0c */	&&inv, &&inv, &&inv, &&inv,
  /* 0x10 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x14 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x18 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x1c */	&&inv, &&inv, &&inv, &&inv,
  /* 0x20 */	&&clz, &&inv, &&inv, &&inv,
  /* 0x24 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x28 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x2c */	&&inv, &&inv, &&inv, &&inv,
  /* 0x30 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x34 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x38 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x3c */	&&inv, &&inv, &&inv, &&inv,
};

static const void * regimm_table[64] = {
  /* 0x00 */    &&bltz, &&bgez, &&bltzl, &&bgezl,
  /* 0x04 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x08 */	&&tgei, &&tgeiu, &&tlti, &&tltiu,
  /* 0x0c */	&&teqi, &&inv, &&tnei, &&inv,
  /* 0x10 */	&&bltzal, &&bgezal, &&bltzall, &&bgezall,
  /* 0x14 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x18 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x1c */	&&inv, &&inv, &&inv, &&inv,
};


static const void * cop0_table_rs[32] = {
  /* 0x00 */    &&mfc0, &&inv, &&inv, &&inv,
  /* 0x04 */    &&mtc0, &&inv, &&inv, &&inv,
  /* 0x08 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x0c */    &&inv, &&inv, &&inv, &&inv,
  /* 0x10 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x14 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x18 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x1c */    &&inv, &&inv, &&inv, &&inv,
};

static const void * cop0_table_func[64] = {
  /* 0x00 */    &&inv, &&tlbr, &&tlbwi, &&inv,
  /* 0x04 */    &&inv, &&inv, &&tlbwr, &&inv,
  /* 0x08 */    &&tlbp, &&inv, &&inv, &&inv,
  /* 0x0c */    &&inv, &&inv, &&inv, &&inv,
  /* 0x10 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x14 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x18 */    &&eret, &&inv, &&inv, &&inv,
  /* 0x1c */    &&inv, &&inv, &&inv, &&inv,

  /* 0x20 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x24 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x28 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x2c */    &&inv, &&inv, &&inv, &&inv,
  /* 0x30 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x34 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x38 */    &&inv, &&inv, &&inv, &&inv,
  /* 0x3c */    &&inv, &&inv, &&inv, &&inv,
};

static const void * opcode_table[64] = {
  /* 0x00 */    &&exec_special, &&exec_regimm, &&j, &&jal,
  /* 0x04 */	&&beq, &&bne, &&blez, &&bgtz,
  /* 0x08 */	&&addi, &&addiu, &&slti, &&sltiu,
  /* 0x0c */	&&andi, &&ori, &&xori, &&lui,
  /* 0x10 */	&&exec_cop0, &&inv, &&inv, &&inv,
  /* 0x14 */	&&beql, &&bnel, &&blezl, &&bgtzl,
  /* 0x18 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x1c */	&&exec_special2, &&inv, &&inv, &&inv,
  /* 0x20 */	&&lb, &&lh, &&lwl, &&lw,
  /* 0x24 */	&&lbu, &&lhu, &&lwr, &&inv,
  /* 0x28 */	&&sb, &&sh, &&swl, &&sw,
  /* 0x2c */	&&inv, &&inv, &&swr, &&cache,
  /* 0x30 */	&&ll, &&inv, &&inv, &&inv,
  /* 0x34 */	&&inv, &&inv, &&inv, &&inv,
  /* 0x38 */	&&sc, &&inv, &&inv, &&inv,
  /* 0x3c */	&&inv, &&inv, &&inv, &&inv,
};

make_entry() {
  goto *opcode_table[inst.op];
}

make_exec_handler(exec_special) ({
  goto *special_table[inst.func];
});

make_exec_handler(exec_special2) ({
  goto *special2_table[inst.func];
});

make_exec_handler(exec_regimm) ({
  goto *regimm_table[inst.rt];
});

make_exec_handler(exec_cop0) ({
  if(inst.rs & 0x10)
	goto *cop0_table_func[inst.func];
  else
	goto *cop0_table_rs[inst.rs];
});


make_exec_handler(inv) ({
  // the pc corresponding to this inst
  // pc has been updated by instr_fetch
  uint32_t ori_pc = cpu.pc - 4;
  uint8_t *p = (uint8_t *)&inst;
  printf("invalid opcode(pc = 0x%08x): %02x %02x %02x %02x ...\n",
	  ori_pc, p[0], p[1], p[2], p[3]);
  nemu_state = NEMU_END;
});


/* tlb strategy */
make_exec_handler(tlbp) ({
  tlb_present();
});

make_exec_handler(tlbr) ({
  tlb_read(inst);
});

make_exec_handler(tlbwi) ({
  tlb_write_by_index(inst);
});

make_exec_handler(tlbwr) ({
  tlb_write_randomly(inst);
});

/* temporary strategy: store timer registers in C0 */
make_exec_handler(syscall) ({
  signal_exception(EXC_SYSCALL);
  dsprintf(asm_buf_p, "syscall");
});

make_exec_handler(breakpoint) ({
  nemu_state = NEMU_STOP;
});

make_exec_handler(eret) ({
  cpu.pc = cpu.cp0.epc;
  cpu.cp0.status.EXL = 0;
  cpu.cp0.status.IE = 1;

#ifdef ENABLE_SEGMENT
  cpu.base = cpu.cp0.reserved[CP0_RESERVED_BASE];
#endif

  dsprintf(asm_buf_p, "eret");
});

make_exec_handler(mfc0) ({
#ifdef ENABLE_INTR
  cpu.gpr[inst.rt] = cpu.cp0.cpr[inst.rd][inst.sel];
#else
  // only for microbench
  if(inst.rd == CP0_COUNT) {
    union { struct { uint32_t lo, hi; }; uint64_t val; } us;
    us.val = get_current_time() * 50; // for 50 MHZ
	if(inst.sel == 0) {
      cpu.gpr[inst.rt] = us.lo;
	} else if(inst.sel == 1) {
      cpu.gpr[inst.rt] = us.hi;
	} else {
	  assert(0);
	}
  } else {
    cpu.gpr[inst.rt] = cpu.cp0.cpr[inst.rd][inst.sel];
  }
#endif
  dsprintf(asm_buf_p, "mfc0 $%s, $%d, %d", regs[inst.rt],
	  inst.rd, inst.sel);
});

make_exec_handler(mtc0) ({
  cpu.cp0.cpr[inst.rd][inst.sel] = cpu.gpr[inst.rt];
  // this serial is for debugging,
  // please don't use it in real codes
  if(inst.rd == CP0_RESERVED && inst.sel == CP0_RESERVED_SERIAL)
    putchar(cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "mtc0 $%s, $%d, %d", regs[inst.rt],
	  inst.rd, inst.sel);
});



make_exec_handler(teq) ({
  if((int32_t)cpu.gpr[inst.rs] == (int32_t)cpu.gpr[inst.rt]) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "teq $%s, $%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(teqi) ({
  if((int32_t)cpu.gpr[inst.rs] == inst.simm) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "teqi $%s, %d", regs[inst.rs], inst.simm);
});

make_exec_handler(tge) ({
  if((int32_t)cpu.gpr[inst.rs] >= (int32_t)cpu.gpr[inst.rt]) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tge $%s, $%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(tgei) ({
  if((int32_t)cpu.gpr[inst.rs] >= inst.simm) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tgei $%s, %d", regs[inst.rs], inst.simm);
});

make_exec_handler(tgeiu) ({
  if(cpu.gpr[inst.rs] >= inst.simm) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tgeiu $%s, %u", regs[inst.rs], inst.simm);
});

make_exec_handler(tgeu) ({
  if(cpu.gpr[inst.rs] >= cpu.gpr[inst.rt]) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tgeu $%s, $%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(tlt) ({
  if((int32_t)cpu.gpr[inst.rs] < (int32_t)cpu.gpr[inst.rt]) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tlt $%s, $%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(tlti) ({
  if((int32_t)cpu.gpr[inst.rs] < inst.simm) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tlti $%s, %d", regs[inst.rs], inst.simm);
});

make_exec_handler(tltiu) ({
  if(cpu.gpr[inst.rs] < inst.simm) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tltiu $%s, %u", regs[inst.rs], inst.simm);
});

make_exec_handler(tltu) ({
  if(cpu.gpr[inst.rs] < cpu.gpr[inst.rt]) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tltu $%s, $%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(tne) ({
  if((int32_t)cpu.gpr[inst.rs] != (int32_t)cpu.gpr[inst.rt]) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tne $%s, $%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(tnei) ({
  if((int32_t)cpu.gpr[inst.rs] != inst.simm) {
    signal_exception(EXC_TRAP);
  }
  dsprintf(asm_buf_p, "tnei $%s, %d", regs[inst.rs], inst.simm);
});


make_exec_handler(jr) ({
  assert(inst.rt == 0 && inst.rd == 0);
  cpu.pc = cpu.gpr[inst.rs];
  exec_delayslot();
  dsprintf(asm_buf_p, "jr %s", regs[inst.rs]);
});

#define R_SIMPLE(name, op, t)                              \
make_exec_handler(name) ({                        \
  assert(inst.shamt == 0);                                 \
  cpu.gpr[inst.rd] = (t)cpu.gpr[inst.rs] op                \
  (t)cpu.gpr[inst.rt];                                     \
  dsprintf(asm_buf_p, "%s %s,%s,%s", #name, regs[inst.rd],  \
	  regs[inst.rs], regs[inst.rt]);                       \
});

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

make_exec_handler(nor) ({
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = ~(cpu.gpr[inst.rs] | cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "nor %s,%s,%s", regs[inst.rd],
	  regs[inst.rs], regs[inst.rt]);
});

#undef R_SIMPLE

make_exec_handler(clz) ({
  if(cpu.gpr[inst.rs] == 0) {
	cpu.gpr[inst.rd] = 32;
  } else {
	cpu.gpr[inst.rd] = __builtin_clz(cpu.gpr[inst.rs]);
  }
  dsprintf(asm_buf_p, "clz %s,%s", regs[inst.rd], regs[inst.rs]);
});

make_exec_handler(mult) ({
  assert(inst.rd == 0 && inst.shamt == 0);
  int64_t prod = (int64_t)(int32_t)cpu.gpr[inst.rs] * (int64_t)(int32_t)cpu.gpr[inst.rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
  dsprintf(asm_buf_p, "mult %s,%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(multu) ({
  assert(inst.rd == 0 && inst.shamt == 0);
  uint64_t prod = (uint64_t)cpu.gpr[inst.rs] * (uint64_t)cpu.gpr[inst.rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
  dsprintf(asm_buf_p, "multu %s,%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(divide) ({
  assert(inst.rd == 0 && inst.shamt == 0);
  cpu.lo = (int32_t)cpu.gpr[inst.rs] / (int32_t)cpu.gpr[inst.rt];
  cpu.hi = (int32_t)cpu.gpr[inst.rs] % (int32_t)cpu.gpr[inst.rt];
  dsprintf(asm_buf_p, "div %s,%s", regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(divu) ({
  assert(inst.rd == 0 && inst.shamt == 0);
  cpu.lo = cpu.gpr[inst.rs] / cpu.gpr[inst.rt];
  cpu.hi = cpu.gpr[inst.rs] % cpu.gpr[inst.rt];
  dsprintf(asm_buf_p, "divu %s,%s", regs[inst.rs], regs[inst.rt]);
});


make_exec_handler(sll) ({
  assert(inst.rs == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] << inst.shamt;
  dsprintf(asm_buf_p, "sll %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
});

make_exec_handler(sllv) ({
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] << (cpu.gpr[inst.rs] & 0x1f);
  dsprintf(asm_buf_p, "sllv %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
});

make_exec_handler(sra) ({
  assert(inst.rs == 0);
  cpu.gpr[inst.rd] = (int32_t)cpu.gpr[inst.rt] >> inst.shamt;
  dsprintf(asm_buf_p, "sra %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
});

make_exec_handler(srav) ({
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = (int32_t)cpu.gpr[inst.rt] >> (cpu.gpr[inst.rs] & 0x1f);
  dsprintf(asm_buf_p, "srav %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
});

make_exec_handler(srl) ({
  assert(inst.rs == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] >> inst.shamt;
  dsprintf(asm_buf_p, "srl %s,%s,0x%x", regs[inst.rd], regs[inst.rt], inst.shamt);
});

make_exec_handler(srlv) ({
  assert(inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.gpr[inst.rt] >> (cpu.gpr[inst.rs] & 0x1f);
  dsprintf(asm_buf_p, "srlv %s,%s,%s", regs[inst.rd], regs[inst.rt], regs[inst.rs]);
});

make_exec_handler(movn) ({
  assert(inst.shamt == 0);
  if (cpu.gpr[inst.rt] != 0)
	cpu.gpr[inst.rd] = cpu.gpr[inst.rs];
  dsprintf(asm_buf_p, "movn %s,%s,%s", regs[inst.rd], regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(movz) ({
  assert(inst.shamt == 0);
  if (cpu.gpr[inst.rt] == 0)
	cpu.gpr[inst.rd] = cpu.gpr[inst.rs];
  dsprintf(asm_buf_p, "movz %s,%s,%s", regs[inst.rd], regs[inst.rs], regs[inst.rt]);
});

make_exec_handler(mfhi) ({
  assert(inst.rs == 0 && inst.rt == 0 && inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.hi;
  dsprintf(asm_buf_p, "mfhi %s", regs[inst.rd]);
});

make_exec_handler(mflo) ({
  assert(inst.rs == 0 && inst.rt == 0 && inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.lo;
  dsprintf(asm_buf_p, "mflo %s", regs[inst.rd]);
});

make_exec_handler(jalr) ({
  assert(inst.rt == 0 && inst.shamt == 0);
  cpu.gpr[inst.rd] = cpu.pc + 4;
  cpu.pc = cpu.gpr[inst.rs];
  exec_delayslot();
  dsprintf(asm_buf_p, "jalr %s,%s", regs[inst.rd], regs[inst.rs]);
});


make_exec_handler(lui) ({
  assert(inst.rs == 0);
  cpu.gpr[inst.rt] = inst.uimm << 16;
  dsprintf(asm_buf_p, "lui %s, 0x%x", regs[inst.rt], inst.uimm);
});

make_exec_handler(addi) ({
  // should throw exception
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] + inst.simm;
  dsprintf(asm_buf_p, "addi %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
});

make_exec_handler(addiu) ({
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] + inst.simm;
  dsprintf(asm_buf_p, "addiu %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
});

make_exec_handler(andi) ({
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] & inst.uimm;
  dsprintf(asm_buf_p, "andi %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.uimm);
});

make_exec_handler(ori) ({
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] | inst.uimm;
  dsprintf(asm_buf_p, "ori %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.uimm);
});

make_exec_handler(xori) ({
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] ^ inst.uimm;
  dsprintf(asm_buf_p, "xori %s, %s, 0x%x", regs[inst.rt], regs[inst.rs], inst.uimm);
});

make_exec_handler(sltiu) ({
  cpu.gpr[inst.rt] = cpu.gpr[inst.rs] < inst.simm;
  dsprintf(asm_buf_p, "sltiu %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
});

make_exec_handler(slti) ({
  cpu.gpr[inst.rt] = (int32_t)cpu.gpr[inst.rs] < inst.simm;
  dsprintf(asm_buf_p, "slti %s, %s, %d", regs[inst.rt], regs[inst.rs], inst.simm);
});


#define CHECK_ALIGNED_ADDR(align, addr) \
  CPUAssert(((addr) & (align - 1)) == 0, "address(0x%08x) is unaligned, pc=%08x\n", (addr), cpu.pc)

make_exec_handler(swl) ({
  uint32_t waddr = cpu.gpr[inst.rs] + inst.simm;
  int idx = waddr & 0x3;
  int len = idx + 1;
  uint32_t wdata = cpu.gpr[inst.rt] >> ((3 - idx) * 8);

  store_mem((waddr >> 2) << 2, len, wdata);
  dsprintf(asm_buf_p, "swl %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(swr) ({
  uint32_t waddr = cpu.gpr[inst.rs] + inst.simm;
  int len = 4 - (waddr & 0x3);
  uint32_t wdata = cpu.gpr[inst.rt];

  store_mem(waddr, len, wdata);
  dsprintf(asm_buf_p, "swr %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(sw) ({
  CHECK_ALIGNED_ADDR(4, cpu.gpr[inst.rs] + inst.simm);
  store_mem(cpu.gpr[inst.rs] + inst.simm, 4, cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "sw %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(sh) ({
  CHECK_ALIGNED_ADDR(2, cpu.gpr[inst.rs] + inst.simm);
  store_mem(cpu.gpr[inst.rs] + inst.simm, 2, cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "sh %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(sb) ({
  CHECK_ALIGNED_ADDR(1, cpu.gpr[inst.rs] + inst.simm);
  store_mem(cpu.gpr[inst.rs] + inst.simm, 1, cpu.gpr[inst.rt]);
  dsprintf(asm_buf_p, "sb %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lwl) ({
  uint32_t raddr = cpu.gpr[inst.rs] + inst.simm;
  int len = (raddr & 0x3) + 1;
  uint32_t rdata = load_mem((raddr >> 2) << 2, len);

  if (len < 4)
	cpu.gpr[inst.rt] = rdata << ((4 - len) * 8) | ((uint32_t)cpu.gpr[inst.rt] << (len * 8)) >> (len * 8);
  else
	cpu.gpr[inst.rt] = rdata;
  dsprintf(asm_buf_p, "lwl %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lwr) ({
  uint32_t raddr = cpu.gpr[inst.rs] + inst.simm;
  int idx = raddr & 0x3;
  int len = 4 - idx;
  uint32_t rdata = load_mem(raddr, len);
  if (len < 4)
	cpu.gpr[inst.rt] = (rdata << idx * 8) >> (idx * 8) | ((uint32_t)cpu.gpr[inst.rt] >> (len * 8)) << (len * 8);
  else
	cpu.gpr[inst.rt] = (rdata << idx * 8) >> (idx * 8);
  dsprintf(asm_buf_p, "lwr %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lw) ({
  CHECK_ALIGNED_ADDR(4, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = load_mem(cpu.gpr[inst.rs] + inst.simm, 4);
  dsprintf(asm_buf_p, "lw %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lb) ({
  CHECK_ALIGNED_ADDR(1, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = (int32_t)(int8_t)load_mem(cpu.gpr[inst.rs] + inst.simm, 1);
  dsprintf(asm_buf_p, "lb %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lbu) ({
  CHECK_ALIGNED_ADDR(1, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = load_mem(cpu.gpr[inst.rs] + inst.simm, 1);
  dsprintf(asm_buf_p, "lbu %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lh) ({
  CHECK_ALIGNED_ADDR(2, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = (int32_t)(int16_t)load_mem(cpu.gpr[inst.rs] + inst.simm, 2);
  dsprintf(asm_buf_p, "lh %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(lhu) ({
  CHECK_ALIGNED_ADDR(2, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = load_mem(cpu.gpr[inst.rs] + inst.simm, 2);
  dsprintf(asm_buf_p, "lhu %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(ll) ({
  CHECK_ALIGNED_ADDR(4, cpu.gpr[inst.rs] + inst.simm);
  cpu.gpr[inst.rt] = vaddr_read(cpu.gpr[inst.rs] + inst.simm, 4);
  dsprintf(asm_buf_p, "ll %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(sc) ({
  CHECK_ALIGNED_ADDR(4, cpu.gpr[inst.rs] + inst.simm);
  vaddr_write(cpu.gpr[inst.rs] + inst.simm, 4, cpu.gpr[inst.rt]);
  cpu.gpr[inst.rt] = 1;
  dsprintf(asm_buf_p, "sc %s, %d(%s)", regs[inst.rt], inst.simm, regs[inst.rs]);
});

make_exec_handler(cache) ({
  dsprintf(asm_buf_p, "cache", regs[inst.rt], inst.simm, regs[inst.rs]);
});


//////////////////////////////////////////////////////////////
//                      likely branch                       //
//////////////////////////////////////////////////////////////
make_exec_handler(beql) ({
  if (cpu.gpr[inst.rs] == cpu.gpr[inst.rt]) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
  dsprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.simm);
});

make_exec_handler(bnel) ({
  if (cpu.gpr[inst.rs] != cpu.gpr[inst.rt]) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
  dsprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.simm);
});

make_exec_handler(blezl) ({
  assert(inst.rt == 0);
  if ((int32_t)cpu.gpr[inst.rs] <= 0) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
  dsprintf(asm_buf_p, "blez %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bgtzl) ({
  if ((int32_t)cpu.gpr[inst.rs] > 0) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
  dsprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bltzl) ({
  if ((int32_t)cpu.gpr[inst.rs] < 0) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
  dsprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bgezl) ({
  if ((int32_t)cpu.gpr[inst.rs] >= 0) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
  dsprintf(asm_buf_p, "bgez %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bgezall) ({
  cpu.gpr[31] = cpu.pc + 4;
  if ((int32_t)cpu.gpr[inst.rs] >= 0) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
});

make_exec_handler(bltzall) ({
  cpu.gpr[31] = cpu.pc + 4;
  if ((int32_t)cpu.gpr[inst.rs] < 0) {
	cpu.pc += inst.simm << 2;
	exec_delayslot();
  } else {
    cpu.pc += 4;
  }
});


//////////////////////////////////////////////////////////////
//                      unlikely branch                    //
//////////////////////////////////////////////////////////////
make_exec_handler(beq) ({
  if (cpu.gpr[inst.rs] == cpu.gpr[inst.rt])
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.simm);
});

make_exec_handler(bne) ({
  if (cpu.gpr[inst.rs] != cpu.gpr[inst.rt])
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "beq %s,%s,0x%x", regs[inst.rs], regs[inst.rt], inst.simm);
});

make_exec_handler(blez) ({
  assert(inst.rt == 0);
  if ((int32_t)cpu.gpr[inst.rs] <= 0)
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "blez %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bgtz) ({
  if ((int32_t)cpu.gpr[inst.rs] > 0)
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bltz) ({
  if ((int32_t)cpu.gpr[inst.rs] < 0)
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "bltz %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bgez) ({
  if ((int32_t)cpu.gpr[inst.rs] >= 0)
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "bgez %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bgezal) ({
  cpu.gpr[31] = cpu.pc + 4;
  if ((int32_t)cpu.gpr[inst.rs] >= 0)
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "bgezal %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(bltzal) ({
  cpu.gpr[31] = cpu.pc + 4;
  if ((int32_t)cpu.gpr[inst.rs] < 0)
	cpu.pc += inst.simm << 2;
  else
    cpu.pc += 4;
  exec_delayslot();
  dsprintf(asm_buf_p, "bltzal %s,0x%x", regs[inst.rs], inst.simm);
});

make_exec_handler(jal) ({
  cpu.gpr[31] = cpu.pc + 4;
  cpu.pc = (cpu.pc & 0xf0000000) | (inst.addr << 2);
  exec_delayslot();
  dsprintf(asm_buf_p, "jal %x", cpu.pc);
});

make_exec_handler(j) ({
  cpu.pc = (cpu.pc & 0xf0000000) | (inst.addr << 2);
  exec_delayslot();
  dsprintf(asm_buf_p, "j %x", cpu.pc);
});

make_eoe() { } // end of execution
