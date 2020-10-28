#define make_label(l) \
  l:
#define make_entry()
#define make_exit() make_label(exit)

enum {
  FPU_FMT_S = 16,
  FPU_FMT_D = 17,
  FPU_FMT_W = 20,
  FPU_FMT_L = 21,
  FPU_FMT_PS,
  FPU_FMT_OB,
  FPU_FMT_QH,
  FPU_FMT_UW,
  FPU_FMT_UD,
};

/*
 * ABS.D ADD.D ADDIU ADD.S ADDU AND ANDI BC1F BC1T BEQ
 * BGEZ BGEZAL BGTZ BLEZ BLTZ BLTZAL BNE BREAK C.EQ.D
 * C.EQ.S CFC1 C.LE.D C.LE.S C.LT.D C.LT.S CLZ CTC1
 * C.ULE.D C.ULE.S C.ULT.D C.ULT.S C.UN.D C.UN.S
 * CVT.D.S CVT.D.W CVT.S.D CVT.S.W DIV DIV.D DIV.S DIVU
 * EXT INS J JAL JALR JR LB LBU LDC1 LH LHU LL LUI LW
 * LWC1 LWL LWR MADD MADDU MFC1 MFHC1 MFHI MFLO MOV.D
 * MOVF MOVF.D MOVF.S MOVN MOVN.D MOVN.S MOV.S MOVT
 * MOVT.D MOVT.S MOVZ MOVZ.S MSUB MTC1 MTHC1 MTHI MTLO
 * MUL MUL.D MUL.S MULT MULTU NEG.D NOR OR ORI PREF RDHWR
 * ROR RORV SB SC SDC1 SEB SEH SH SLL SLLV SLT SLTI SLTIU
 * SLTU SQRT.D SQRT.S SRA SRAV SRL SRLV SUB.D SUB.S SUBU
 * SW SWC1 SWL SWR SYNC SYSCALL TEQ TRUNC.W.D TRUNC.W.S
 * WSBH XOR XORI
 * */

#if CONFIG_DELAYSLOT
#  define make_exec_handler(name) \
    goto inst_end;                \
    make_label(name)
#  define prepare_delayslot() \
    cpu.is_delayslot = true;  \
    cpu.pc += 4;              \
    goto exit;
#else
#  define make_exec_handler(name) \
    cpu.pc += 4;                  \
    goto exit;                    \
    make_label(name)
#  define prepare_delayslot() \
    cpu.pc = cpu.br_target;   \
    goto exit;
#endif

#if CONFIG_EXCEPTION
#  define InstAssert(cond)         \
    do {                           \
      if (!(cond)) {               \
        cpu.cp0.badvaddr = cpu.pc; \
        launch_exception(EXC_RI);  \
        goto exit;                 \
      }                            \
    } while (0)
#else
#  define InstAssert(cond) assert(cond)
#endif

typedef union {
  struct {
    uint32_t lo, hi;
  };
  uint64_t val;
  int64_t sval;
} L64_t;

/* clang-format off */
/* R-type */
static const void *special_table[64] = {
    /* 0x00 */ &&sll, &&movci, &&srl, &&sra,
    /* 0x04 */ &&sllv, &&inv, &&srlv, &&srav,
    /* 0x08 */ &&jr, &&jalr, &&movz, &&movn,
    /* 0x0c */ &&syscall, &&breakpoint, &&inv, &&sync,
    /* 0x10 */ &&mfhi, &&mthi, &&mflo, &&mtlo,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&mult, &&multu, &&divide, &&divu,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&add, &&addu, &&sub, &&subu,
    /* 0x24 */ &&and, && or, &&xor, &&nor,
    /* 0x28 */ &&inv, &&inv, &&slt, &&sltu,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&tge, &&tgeu, &&tlt, &&tltu,
    /* 0x34 */ &&teq, &&inv, &&tne, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

static const void *special2_table[64] = {
    /* 0x00 */ &&madd, &&maddu, &&mul, &&inv,
    /* 0x04 */ &&msub, &&msubu, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&clz, &&clo, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

static const void *special3_table[64] = {
    /* 0x00 */ &&ext, &&inv, &&mul, &&inv,
    /* 0x04 */ &&ins, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&exec_bshfl, &&inv, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

/* shamt */
static const void *bshfl_table[64] = {
    /* 0x00 */ &&inv, &&inv, &&wsbh, &&inv,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&seb, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&seh, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
};

/* I-type */
static const void *regimm_table[64] = {
    /* 0x00 */ &&bltz, &&bgez, &&bltzl, &&bgezl,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&tgei, &&tgeiu, &&tlti, &&tltiu,
    /* 0x0c */ &&teqi, &&inv, &&tnei, &&inv,
    /* 0x10 */ &&bltzal, &&bgezal, &&bltzall, &&bgezall,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&synci,
};

/* R-type */
static const void *cop0_table_rs[32] = {
    /* 0x00 */ &&mfc0, &&inv, &&inv, &&inv,
    /* 0x04 */ &&mtc0, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv,  &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv,  &&inv, &&inv, &&inv,
};

static const void *cop0_table_func[64] = {
    /* 0x00 */ &&inv,  &&tlbr, &&tlbwi, &&inv,
    /* 0x04 */ &&inv,  &&inv,  &&tlbwr, &&inv,
    /* 0x08 */ &&tlbp, &&inv,  &&inv,   &&inv,
    /* 0x0c */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x10 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x14 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x18 */ &&eret, &&inv,  &&inv,   &&inv,
    /* 0x1c */ &&inv,  &&inv,  &&inv,   &&inv,

    /* 0x20 */ &&wait,  &&inv,  &&inv,   &&inv,
    /* 0x24 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x28 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x2c */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x30 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x34 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x38 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x3c */ &&inv,  &&inv,  &&inv,   &&inv,
};

static const void *cop1_table_rs[32] = {
    /* 0x00 */ &&mfc1, &&inv, &&cfc1, &&mfhc1,
    /* 0x04 */ &&mtc1, &&inv, &&ctc1, &&mthc1,
    /* 0x08 */ &&bc1, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
};

static const void *cop1_table_rs_S[64] = {
    /* 0x00 */ &&add_s, &&sub_s, &&mul_s, &&div_s,
    /* 0x04 */ &&sqrt_s, &&abs_s, &&mov_s, &&neg_s,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&trunc_w_s, &&inv, &&inv,
    /* 0x10 */ &&inv, &&movcf_s, &&movz_s, &&movn_s,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,

    /* 0x20 */ &&inv, &&cvt_d_s, &&inv, &&inv,
    /* 0x24 */ &&inv, &&cvt_w_s, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&c_un_s, &&c_eq_s, &&c_ueq_s,
    /* 0x34 */ &&c_olt_s, &&c_ult_s, &&c_ole_s, &&c_ule_s,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&c_lt_s, &&inv, &&c_le_s, &&inv,
};

static const void *cop1_table_rs_D[64] = {
    /* 0x00 */ &&add_d, &&sub_d, &&mul_d, &&div_d,
    /* 0x04 */ &&sqrt_d, &&abs_d, &&mov_d, &&neg_d,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&trunc_w_d, &&inv, &&inv,
    /* 0x10 */ &&inv, &&movcf_d, &&movz_d, &&movn_d,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,

    /* 0x20 */ &&cvt_s_d, &&inv, &&inv, &&inv,
    /* 0x24 */ &&inv, &&cvt_w_d, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&c_un_d, &&c_eq_d, &&c_ueq_d,
    /* 0x34 */ &&c_olt_d, &&c_ult_d, &&c_ole_d, &&c_ule_d,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&c_lt_d, &&inv, &&c_le_d, &&inv,
};

static const void *cop1_table_rs_W[64] = {
    /* 0x00 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,

    /* 0x20 */ &&cvt_s_w, &&cvt_d_w, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

/* I-type */
static const void *opcode_table[64] = {
    /* 0x00 */ &&exec_special, &&exec_regimm, &&j, &&jal,
    /* 0x04 */ &&beq, &&bne, &&blez, &&bgtz,
    /* 0x08 */ &&addi, &&addiu, &&slti, &&sltiu,
    /* 0x0c */ &&andi, &&ori, &&xori, &&lui,
    /* 0x10 */ &&exec_cop0, &&exec_cop1, &&inv, &&inv,
    /* 0x14 */ &&beql, &&bnel, &&blezl, &&bgtzl,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&exec_special2, &&inv, &&inv, &&exec_special3,
    /* 0x20 */ &&lb, &&lh, &&lwl, &&lw,
    /* 0x24 */ &&lbu, &&lhu, &&lwr, &&inv,
    /* 0x28 */ &&sb, &&sh, &&swl, &&sw,
    /* 0x2c */ &&inv, &&inv, &&swr, &&cache,
    /* 0x30 */ &&ll, &&lwc1, &&inv, &&pref,
    /* 0x34 */ &&inv, &&ldc1, &&inv, &&inv,
    /* 0x38 */ &&sc, &&swc1, &&inv, &&inv,
    /* 0x3c */ &&inv, &&sdc1, &&inv, &&inv,
};

/* clang-format on */
#if CONFIG_DECODE_CACHE
make_entry() {
  cpu.gpr[0] = 0;

#  if CONFIG_DECODE_CACHE_PERF
  decode_cache_hit += !!decode->handler;
  decode_cache_miss += !decode->handler;
#  endif

  if (decode->handler) {
#  if CONFIG_INSTR_LOG
    instr_enqueue_instr(decode->inst.val);
#  endif

#  if CONFIG_DECODE_CACHE_CHECK
    assert(decode->inst.val == dbg_vaddr_read(cpu.pc, 4));
#  endif
    goto *(decode->handler);
  }

  Inst inst = {.val = vaddr_read(cpu.pc, 4)};
#  if CONFIG_INSTR_LOG || CONFIG_DECODE_CACHE_CHECK
  decode->inst.val = inst.val;
#  endif
#  if CONFIG_INSTR_LOG
  instr_enqueue_instr(decode->inst.val);
#  endif

  unsigned op = inst.op;
  switch (op) {
  case 0x00: goto Rtype;
  case 0x01: goto Itype;
  case 0x02:
  case 0x03: goto Jtype;
  case 0x10:
    if (inst.rs & 0x10) {
      decode->handler = cop0_table_func[inst.func];
      goto Handler;
    } else {
      goto Cop0Type;
    }
    break;
  case 0x1c: goto S2type;
  case 0x1f:
    if (inst.func == 0x20)
      goto bshflType;
    else
      goto S3Type;
  default: goto Itype;
  }

  do {
  Rtype : {
    decode->rs = inst.rs;
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->shamt = inst.shamt;
    decode->func = inst.func;
    decode->handler = special_table[inst.func];
    break;
  }
  Itype : {
    decode->rs = inst.rs;
    decode->rt = inst.rt;
    decode->uimm = inst.uimm;
    decode->handler = opcode_table[op];
    break;
  }
  Jtype : {
    decode->addr = inst.addr;
    decode->handler = opcode_table[op];
    break;
  }
  Cop0Type : {
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->sel = inst.sel;
    decode->handler = cop0_table_rs[inst.rs];
    break;
  }
  S2type : {
    decode->rs = inst.rs;
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->shamt = inst.shamt;
    decode->handler = special2_table[inst.func];
    break;
  }
  S3Type : {
    decode->handler = special3_table[inst.func];
    break;
  }
  bshflType : {
    decode->rt = inst.rt;
    decode->rd = inst.rd;
    decode->handler = bshfl_table[inst.shamt];
    break;
  }
  } while (0);

Handler:
  goto *(decode->handler);
}
#else
make_entry() {
  cpu.gpr[0] = 0;
#  if CONFIG_INSTR_LOG
  instr_enqueue_instr(inst.val);
#  endif
  goto *opcode_table[inst.op];
}
#endif

#if 1
make_exec_handler(exec_special) {
  goto *special_table[operands->func];
}
make_exec_handler(exec_special2) {
  goto *special2_table[operands->func];
}
make_exec_handler(exec_special3) {
  goto *special3_table[operands->func];
}
make_exec_handler(exec_bshfl) {
  goto *bshfl_table[operands->shamt];
}

make_exec_handler(exec_regimm) {
  goto *regimm_table[operands->rt];
}

make_exec_handler(exec_cop0) {
  if (operands->rs & 0x10)
    goto *cop0_table_func[operands->func];
  else
    goto *cop0_table_rs[operands->rs];
}

make_exec_handler(exec_cop1) {
  if (operands->rs == FPU_FMT_S)
    goto *cop1_table_rs_S[operands->func];
  else if (operands->rs == FPU_FMT_D)
    goto *cop1_table_rs_D[operands->func];
  else if (operands->rs == FPU_FMT_W)
    goto *cop1_table_rs_W[operands->func];
  else
    goto *cop1_table_rs[operands->rs];
}
#endif

make_exec_handler(inv) {
// the pc corresponding to this inst
// pc has been updated by instr_fetch
#if CONFIG_EXCEPTION
  launch_exception(EXC_RI);
#else
  uint32_t instr = vaddr_read(cpu.pc, 4);
  uint8_t *p = (uint8_t *)&instr;
  printf(
      "invalid opcode(pc = 0x%08x): %02x %02x %02x %02x "
      "...\n",
      cpu.pc, p[0], p[1], p[2], p[3]);
  nemu_state = NEMU_END;
#endif
}

/* tlb strategy */
make_exec_handler(tlbp) { tlb_present(); }

make_exec_handler(tlbr) {
  uint32_t i = cpu.cp0.index.idx;
  CPUAssert(i < NR_TLB_ENTRY, "invalid tlb index\n");
  tlb_read(i);
}

make_exec_handler(tlbwi) {
  uint32_t i = cpu.cp0.index.idx;
  CPUAssert(i < NR_TLB_ENTRY, "invalid tlb index %d (%d)\n",
      i, NR_TLB_ENTRY);
  tlb_write(i);
  clear_mmu_cache();
  clear_decode_cache();
}

make_exec_handler(tlbwr) {
#if CONFIG_MARCH_NOOP
  tlb_write(cpu.cp0.random);
  cpu.cp0.random = (cpu.cp0.random + 1) % NR_TLB_ENTRY;
#else
  cpu.cp0.random = rand() % NR_TLB_ENTRY;
  tlb_write(cpu.cp0.random);
#endif
  clear_mmu_cache();
  clear_decode_cache();
}

/* temporary strategy: store timer registers in C0 */
make_exec_handler(syscall) {
  launch_exception(EXC_SYSCALL);
#if CONFIG_DUMP_SYSCALL
  cpu.is_syscall = true;
  void dump_syscall(
      uint32_t v0, uint32_t a0, uint32_t a1, uint32_t a2);
  dump_syscall(cpu.gpr[R_v0], cpu.gpr[R_a0], cpu.gpr[R_a1],
      cpu.gpr[R_a2]);
#endif
}

make_exec_handler(breakpoint) {
  if (work_mode == MODE_GDB) {
    nemu_state = NEMU_STOP;
  } else {
    launch_exception(EXC_BP);
  }
}

make_exec_handler(wait) { /* didn't +4 for pc */
  usleep(1);
}

make_exec_handler(eret) {
  cpu.has_exception = true;

  if (cpu.cp0.status.ERL == 1) {
    cpu.br_target = cpu.cp0.cpr[CP0_ErrorEPC][0];
    cpu.cp0.status.ERL = 0;
  } else {
    cpu.br_target = cpu.cp0.epc;
    cpu.cp0.status.EXL = 0;
  }

#if 0
  eprintf("%08x: ERET to %08x: ERL %d, EXL %d\n", cpu.pc, cpu.br_target,
      cpu.cp0.status.ERL, cpu.cp0.status.EXL);
#endif

#if CONFIG_DUMP_SYSCALL
  if (cpu.is_syscall) {
    printf("==> v0: %08x & %d\n", cpu.gpr[R_v0],
        cpu.gpr[R_v0]);
    cpu.is_syscall = false;
  }
#endif

#if CONFIG_CAE_CHECK
  check_usual_registers();
#endif

#if CONFIG_SEGMENT
  cpu.base = cpu.cp0.reserved[CP0_RESERVED_BASE];
#endif

  clear_mmu_cache();
  clear_decode_cache();
}

#define CPRS(reg, sel) (((reg) << 3) | (sel))

make_exec_handler(mfc0) {
  /* used for nanos: pal and litenes */
  if (operands->rd == CP0_COUNT) {
    cpu.gpr[operands->rt] = cpu.cp0.count[0];
  } else {
    cpu.gpr[operands->rt] =
        cpu.cp0.cpr[operands->rd][operands->sel];
  }
}

make_exec_handler(mtc0) {
  switch (CPRS(operands->rd, operands->sel)) {
  case CPRS(CP0_EBASE, CP0_EBASE_SEL):
  case CPRS(CP0_COUNT, 0):
  case CPRS(CP0_EPC, 0):
    cpu.cp0.cpr[operands->rd][operands->sel] =
        cpu.gpr[operands->rt];
    break;
  case CPRS(CP0_BADVADDR, 0): break;
  case CPRS(CP0_CONTEXT, 0): {
    cp0_context_t *newVal =
        (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.context.PTEBase = newVal->PTEBase;
  } break;
  case CPRS(CP0_CONFIG, 0): {
    cp0_config_t *newVal = (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.config.K0 = newVal->K0;
  } break;
  case CPRS(CP0_STATUS, 0): {
    cp0_status_t *newVal = (void *)&(cpu.gpr[operands->rt]);
    if (cpu.cp0.status.ERL != newVal->ERL) {
      clear_decode_cache();
      clear_mmu_cache();
    }
    cpu.cp0.status.CU = newVal->CU & 0x3;
    cpu.cp0.status.RP = newVal->RP;
    cpu.cp0.status.RE = newVal->RE;
    cpu.cp0.status.BEV = newVal->BEV;
    cpu.cp0.status.TS = newVal->TS;
    cpu.cp0.status.SR = newVal->SR;
    cpu.cp0.status.NMI = newVal->NMI;
    cpu.cp0.status.IM = newVal->IM;
    cpu.cp0.status.UM = newVal->UM;
    cpu.cp0.status.ERL = newVal->ERL;
    cpu.cp0.status.EXL = newVal->EXL;
    cpu.cp0.status.IE = newVal->IE;
  } break;
  case CPRS(CP0_COMPARE, 0):
    cpu.cp0.compare = cpu.gpr[operands->rt];
    nemu_set_irq(7, 0);
    break;
  case CPRS(CP0_CAUSE, 0): {
    cp0_cause_t *newVal = (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.cause.IV = newVal->IV;
    cpu.cp0.cause.WP = newVal->WP;

    nemu_set_irq(0, newVal->IP & (1 << 0));
    nemu_set_irq(1, newVal->IP & (1 << 1));
  } break;
  case CPRS(CP0_PAGEMASK, 0): {
    cp0_pagemask_t *newVal =
        (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.pagemask.mask = newVal->mask;
    break;
  }
  case CPRS(CP0_ENTRY_LO0, 0): {
    cp0_entry_lo_t *newVal =
        (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.entry_lo0.g = newVal->g;
    cpu.cp0.entry_lo0.v = newVal->v;
    cpu.cp0.entry_lo0.d = newVal->d;
    cpu.cp0.entry_lo0.c = newVal->c;
    cpu.cp0.entry_lo0.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_LO1, 0): {
    cp0_entry_lo_t *newVal =
        (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.entry_lo1.g = newVal->g;
    cpu.cp0.entry_lo1.v = newVal->v;
    cpu.cp0.entry_lo1.d = newVal->d;
    cpu.cp0.entry_lo1.c = newVal->c;
    cpu.cp0.entry_lo1.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_HI, 0): {
    cp0_entry_hi_t *newVal =
        (void *)&(cpu.gpr[operands->rt]);
    cpu.cp0.entry_hi.asid = newVal->asid;
    cpu.cp0.entry_hi.vpn = newVal->vpn;
    clear_mmu_cache();
    clear_decode_cache();
  } break;
  case CPRS(CP0_INDEX, 0): {
    cpu.cp0.index.idx = cpu.gpr[operands->rt];
  } break;
  // this serial is for debugging,
  // please don't use it in real codes
  case CPRS(CP0_RESERVED, CP0_RESERVED_BASE):
#if CONFIG_SEGMENT
    cpu.cp0.cpr[operands->rd][operands->sel] =
        cpu.gpr[operands->rt];
#endif
    break;
  case CPRS(CP0_RESERVED, CP0_RESERVED_SERIAL): {
#if CONFIG_KERNEL_DEBUG_SERIAL
    putchar(cpu.gpr[operands->rt]);
#endif
    break;
  }
  case CPRS(CP0_RESERVED, CP0_RESERVED_CHECK): {
#if CONFIG_CHECK_IMAGE
    extern const char *symbol_file;
    check_kernel_image(symbol_file);
#endif
    break;
  }
  case CPRS(CP0_RESERVED, CP0_RESERVED_PRINT_REGISTERS): {
#if CONFIG_INSTR_LOG
    // kdbg_print_registers();
#endif
  } break;
  case CPRS(CP0_RESERVED, CP0_RESERVED_PRINT_INSTR_QUEUE): {
#if CONFIG_INSTR_LOG
    // kdbg_print_instr_queue();
#endif
  } break;
  case CPRS(CP0_RESERVED, CP0_RESERVED_TOGGLE_COMMITS): {
#if CONFIG_INSTR_LOG
    // nemu_needs_commit = !nemu_needs_commit;
#endif
  } break;
  case CPRS(CP0_RESERVED, CP0_RESERVED_HIT_TRAP): {
    if (cpu.gpr[operands->rt] == 0)
      printf("\e[1;32mHIT GOOD TRAP\e[0m\n");
    else
      printf("\e[1;31mHIT BAD TRAP %d\e[0m\n",
          cpu.gpr[operands->rt]);
    nemu_exit(0);
  } break;
  default:
    printf("%08x: mtc0 $%s, $%d, %d\n", cpu.pc,
        regs[operands->rt], operands->rd, operands->sel);
    break;
  }
}

make_exec_handler(teq) {
  if ((int32_t)cpu.gpr[operands->rs] ==
      (int32_t)cpu.gpr[operands->rt]) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(teqi) {
  if ((int32_t)cpu.gpr[operands->rs] == operands->simm) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tge) {
  if ((int32_t)cpu.gpr[operands->rs] >=
      (int32_t)cpu.gpr[operands->rt]) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tgei) {
  if ((int32_t)cpu.gpr[operands->rs] >= operands->simm) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tgeiu) {
  if (cpu.gpr[operands->rs] >= operands->simm) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tgeu) {
  if (cpu.gpr[operands->rs] >= cpu.gpr[operands->rt]) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tlt) {
  if ((int32_t)cpu.gpr[operands->rs] <
      (int32_t)cpu.gpr[operands->rt]) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tlti) {
  if ((int32_t)cpu.gpr[operands->rs] < operands->simm) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tltiu) {
  if (cpu.gpr[operands->rs] < operands->simm) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tltu) {
  if (cpu.gpr[operands->rs] < cpu.gpr[operands->rt]) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tne) {
  if ((int32_t)cpu.gpr[operands->rs] !=
      (int32_t)cpu.gpr[operands->rt]) {
    launch_exception(EXC_TRAP);
  }
}

make_exec_handler(tnei) {
  if ((int32_t)cpu.gpr[operands->rs] != operands->simm) {
    launch_exception(EXC_TRAP);
  }
}

#define R_SIMPLE(name, op, t)          \
  make_exec_handler(name) {            \
    InstAssert(operands->shamt == 0);  \
    cpu.gpr[operands->rd] =            \
        (t)cpu.gpr[operands->rs] op(t) \
            cpu.gpr[operands->rt];     \
  }

R_SIMPLE(or, |, uint32_t)
R_SIMPLE(xor, ^, uint32_t)
R_SIMPLE(and, &, uint32_t)
R_SIMPLE(addu, +, uint32_t)
R_SIMPLE(subu, -, uint32_t)
R_SIMPLE(mul, *, uint32_t)
R_SIMPLE(slt, <, int32_t)
R_SIMPLE(sltu, <, uint32_t)

make_exec_handler(add) {
  InstAssert(operands->shamt == 0);
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[operands->rs] +
            (int64_t)(int32_t)cpu.gpr[operands->rt];
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    launch_exception(EXC_OV);
#else
    CPUAssert(0, "add overflow, %08x + %08x\n",
        cpu.gpr[operands->rs], cpu.gpr[operands->rt]);
#endif
  } else {
    cpu.gpr[operands->rd] = ret.lo;
  }
}

make_exec_handler(sub) {
  InstAssert(operands->shamt == 0);
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[operands->rs] -
            (int64_t)(int32_t)cpu.gpr[operands->rt];
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    launch_exception(EXC_OV);
#else
    CPUAssert(0, "sub overflow, %08x - %08x\n",
        cpu.gpr[operands->rs], cpu.gpr[operands->rt]);
#endif
  } else {
    cpu.gpr[operands->rd] = ret.lo;
  }
}

make_exec_handler(nor) {
  InstAssert(operands->shamt == 0);
  cpu.gpr[operands->rd] =
      ~(cpu.gpr[operands->rs] | cpu.gpr[operands->rt]);
}

#undef R_SIMPLE

make_exec_handler(clz) {
  if (cpu.gpr[operands->rs] == 0) {
    cpu.gpr[operands->rd] = 32;
  } else {
    cpu.gpr[operands->rd] =
        __builtin_clz(cpu.gpr[operands->rs]);
  }
}

make_exec_handler(clo) {
  uint32_t in = cpu.gpr[operands->rs];
  uint32_t cnt = 0;
  uint32_t b = 0x80000000;
  while ((in & b) != 0) {
    cnt++;
    b >>= 1;
  }
  cpu.gpr[operands->rd] = cnt;
}

make_exec_handler(madd) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  L64_t hilo;
  hilo.hi = cpu.hi;
  hilo.lo = cpu.lo;
  hilo.sval += (int64_t)(int32_t)cpu.gpr[operands->rs] *
               (int64_t)(int32_t)cpu.gpr[operands->rt];
  cpu.hi = hilo.hi;
  cpu.lo = hilo.lo;
}

make_exec_handler(maddu) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  L64_t hilo;
  hilo.hi = cpu.hi;
  hilo.lo = cpu.lo;
  hilo.val += (uint64_t)cpu.gpr[operands->rs] *
              (uint64_t)cpu.gpr[operands->rt];
  cpu.hi = hilo.hi;
  cpu.lo = hilo.lo;
}

make_exec_handler(msub) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  L64_t hilo;
  hilo.hi = cpu.hi;
  hilo.lo = cpu.lo;
  hilo.sval -= (int64_t)(int32_t)cpu.gpr[operands->rs] *
               (int64_t)(int32_t)cpu.gpr[operands->rt];
  cpu.hi = hilo.hi;
  cpu.lo = hilo.lo;
}

make_exec_handler(msubu) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  L64_t hilo;
  hilo.hi = cpu.hi;
  hilo.lo = cpu.lo;
  hilo.val -= (uint64_t)cpu.gpr[operands->rs] *
              (uint64_t)cpu.gpr[operands->rt];
  cpu.hi = hilo.hi;
  cpu.lo = hilo.lo;
}

make_exec_handler(mult) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  int64_t prod = (int64_t)(int32_t)cpu.gpr[operands->rs] *
                 (int64_t)(int32_t)cpu.gpr[operands->rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
}

make_exec_handler(multu) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  uint64_t prod = (uint64_t)cpu.gpr[operands->rs] *
                  (uint64_t)cpu.gpr[operands->rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
}

make_exec_handler(divide) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  cpu.lo = (int32_t)cpu.gpr[operands->rs] /
           (int32_t)cpu.gpr[operands->rt];
  cpu.hi = (int32_t)cpu.gpr[operands->rs] %
           (int32_t)cpu.gpr[operands->rt];
}

make_exec_handler(divu) {
  InstAssert(operands->rd == 0 && operands->shamt == 0);
  cpu.lo = cpu.gpr[operands->rs] / cpu.gpr[operands->rt];
  cpu.hi = cpu.gpr[operands->rs] % cpu.gpr[operands->rt];
}

make_exec_handler(sll) {
  InstAssert(operands->rs == 0);
  cpu.gpr[operands->rd] = cpu.gpr[operands->rt]
                          << operands->shamt;
}

make_exec_handler(sllv) {
  InstAssert(operands->shamt == 0);
  cpu.gpr[operands->rd] = cpu.gpr[operands->rt]
                          << (cpu.gpr[operands->rs] & 0x1f);
}

make_exec_handler(sra) {
  InstAssert(operands->rs == 0);
  cpu.gpr[operands->rd] =
      (int32_t)cpu.gpr[operands->rt] >> operands->shamt;
}

make_exec_handler(srav) {
  InstAssert(operands->shamt == 0);
  cpu.gpr[operands->rd] = (int32_t)cpu.gpr[operands->rt] >>
                          (cpu.gpr[operands->rs] & 0x1f);
}

make_exec_handler(srl) {
  if ((operands->rs & 0x1) == 0x1) {
    /* rotr */
    uint32_t rt_val = cpu.gpr[operands->rt];
    uint32_t sa = operands->shamt;
    cpu.gpr[operands->rd] =
        (rt_val >> sa) | (rt_val << (32 - sa));
  } else {
    InstAssert(operands->rs == 0);
    cpu.gpr[operands->rd] =
        cpu.gpr[operands->rt] >> operands->shamt;
  }
}

make_exec_handler(srlv) {
  if ((operands->shamt & 0x1) == 0x1) {
    /* rotrv */
    uint32_t rt_val = cpu.gpr[operands->rt];
    uint32_t sa = cpu.gpr[operands->rs] & 0x1f;
    cpu.gpr[operands->rd] =
        (rt_val >> sa) | (rt_val << (32 - sa));
  } else {
    InstAssert(operands->shamt == 0);
    cpu.gpr[operands->rd] = cpu.gpr[operands->rt] >>
                            (cpu.gpr[operands->rs] & 0x1f);
  }
}

make_exec_handler(movn) {
  InstAssert(operands->shamt == 0);
  if (cpu.gpr[operands->rt] != 0)
    cpu.gpr[operands->rd] = cpu.gpr[operands->rs];
}

make_exec_handler(movz) {
  InstAssert(operands->shamt == 0);
  if (cpu.gpr[operands->rt] == 0)
    cpu.gpr[operands->rd] = cpu.gpr[operands->rs];
}

make_exec_handler(mfhi) {
  InstAssert(operands->rs == 0 && operands->rt == 0 &&
             operands->shamt == 0);
  cpu.gpr[operands->rd] = cpu.hi;
}

make_exec_handler(mthi) {
  InstAssert(operands->rt == 0 && operands->rd == 0 &&
             operands->shamt == 0);
  cpu.hi = cpu.gpr[operands->rs];
}

make_exec_handler(mflo) {
  InstAssert(operands->rs == 0 && operands->rt == 0 &&
             operands->shamt == 0);
  cpu.gpr[operands->rd] = cpu.lo;
}

make_exec_handler(mtlo) {
  InstAssert(operands->rt == 0 && operands->rd == 0 &&
             operands->shamt == 0);
  cpu.lo = cpu.gpr[operands->rs];
}

make_exec_handler(lui) {
  InstAssert(operands->rs == 0);
  cpu.gpr[operands->rt] = operands->uimm << 16;
}

make_exec_handler(addi) {
  // should throw exception
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[operands->rs] +
            (int64_t)(int32_t)operands->simm;
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#if CONFIG_EXCEPTION
    launch_exception(EXC_OV);
#else
    CPUAssert(0, "addi overflow, %08x + %08x\n",
        cpu.gpr[operands->rs], operands->simm);
#endif
  } else {
    cpu.gpr[operands->rt] = ret.lo;
  }
}

make_exec_handler(addiu) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] + operands->simm;
}

make_exec_handler(andi) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] & operands->uimm;
}

make_exec_handler(ori) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] | operands->uimm;
}

make_exec_handler(xori) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] ^ operands->uimm;
}

make_exec_handler(sltiu) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] < operands->simm;
}

make_exec_handler(slti) {
  cpu.gpr[operands->rt] =
      (int32_t)cpu.gpr[operands->rs] < operands->simm;
}

#if CONFIG_EXCEPTION

#  define CHECK_ALIGNED_ADDR_AdEL(align, addr) \
    if (((addr) & (align - 1)) != 0) {         \
      cpu.cp0.badvaddr = addr;                 \
      launch_exception(EXC_AdEL);              \
      goto exit;                               \
    }

#  define CHECK_ALIGNED_ADDR_AdES(align, addr) \
    if (((addr) & (align - 1)) != 0) {         \
      cpu.cp0.badvaddr = addr;                 \
      launch_exception(EXC_AdES);              \
      goto exit;                               \
    }

#else

#  define CHECK_ALIGNED_ADDR(align, addr)                  \
    CPUAssert(((addr) & (align - 1)) == 0,                 \
        "address(0x%08x) is unaligned, pc=%08x\n", (addr), \
        cpu.pc)

#  define CHECK_ALIGNED_ADDR_AdEL CHECK_ALIGNED_ADDR
#  define CHECK_ALIGNED_ADDR_AdES CHECK_ALIGNED_ADDR

#endif

make_exec_handler(swl) {
  uint32_t waddr = cpu.gpr[operands->rs] + operands->simm;
  int idx = waddr & 0x3;
  int len = idx + 1;
  uint32_t wdata = cpu.gpr[operands->rt] >> ((3 - idx) * 8);

  vaddr_write((waddr >> 2) << 2, len, wdata);
  if (cpu.has_exception) cpu.cp0.badvaddr = waddr;
}

make_exec_handler(swr) {
  uint32_t waddr = cpu.gpr[operands->rs] + operands->simm;
  int len = 4 - (waddr & 0x3);
  uint32_t wdata = cpu.gpr[operands->rt];

  vaddr_write(waddr, len, wdata);
}

make_exec_handler(sw) {
  CHECK_ALIGNED_ADDR_AdES(
      4, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 4,
      cpu.gpr[operands->rt]);
}

make_exec_handler(sh) {
  CHECK_ALIGNED_ADDR_AdES(
      2, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 2,
      cpu.gpr[operands->rt]);
}

make_exec_handler(sb) {
  CHECK_ALIGNED_ADDR_AdES(
      1, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 1,
      cpu.gpr[operands->rt]);
}

make_exec_handler(lwl) {
  uint32_t raddr = cpu.gpr[operands->rs] + operands->simm;
  int len = (raddr & 0x3) + 1;
  uint32_t rdata = vaddr_read((raddr >> 2) << 2, len);

  if (!cpu.has_exception) {
    if (len < 4)
      cpu.gpr[operands->rt] =
          rdata << ((4 - len) * 8) |
          ((uint32_t)cpu.gpr[operands->rt] << (len * 8)) >>
              (len * 8);
    else
      cpu.gpr[operands->rt] = rdata;
  } else {
    cpu.cp0.badvaddr = raddr;
  }
}

make_exec_handler(lwr) {
  uint32_t raddr = cpu.gpr[operands->rs] + operands->simm;
  int idx = raddr & 0x3;
  int len = 4 - idx;
  uint32_t rdata = vaddr_read(raddr, len);
  if (!cpu.has_exception) {
    if (len < 4)
      cpu.gpr[operands->rt] =
          (rdata << idx * 8) >> (idx * 8) |
          ((uint32_t)cpu.gpr[operands->rt] >> (len * 8))
              << (len * 8);
    else
      cpu.gpr[operands->rt] =
          (rdata << idx * 8) >> (idx * 8);
  }
}

make_exec_handler(lw) {
  CHECK_ALIGNED_ADDR_AdEL(
      4, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 4);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lb) {
  CHECK_ALIGNED_ADDR_AdEL(
      1, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata = (int32_t)(int8_t)vaddr_read(
      cpu.gpr[operands->rs] + operands->simm, 1);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lbu) {
  CHECK_ALIGNED_ADDR_AdEL(
      1, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 1);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lh) {
  CHECK_ALIGNED_ADDR_AdEL(
      2, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata = (int32_t)(int16_t)vaddr_read(
      cpu.gpr[operands->rs] + operands->simm, 2);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(lhu) {
  CHECK_ALIGNED_ADDR_AdEL(
      2, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 2);
  if (!cpu.has_exception) { cpu.gpr[operands->rt] = rdata; }
}

make_exec_handler(pref) {}

make_exec_handler(ll) {
  CHECK_ALIGNED_ADDR_AdEL(
      4, cpu.gpr[operands->rs] + operands->simm);
  cpu.gpr[operands->rt] =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 4);
}

make_exec_handler(sc) {
  CHECK_ALIGNED_ADDR_AdES(
      4, cpu.gpr[operands->rs] + operands->simm);
  vaddr_write(cpu.gpr[operands->rs] + operands->simm, 4,
      cpu.gpr[operands->rt]);
  if (!cpu.has_exception) cpu.gpr[operands->rt] = 1;
}

make_exec_handler(cache) { clear_decode_cache(); }

make_exec_handler(sync) {}
make_exec_handler(synci) {}

//////////////////////////////////////////////////////////////
//                      likely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beql) {
  if (cpu.gpr[operands->rs] == cpu.gpr[operands->rt]) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bnel) {
  if (cpu.gpr[operands->rs] != cpu.gpr[operands->rt]) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(blezl) {
  InstAssert(operands->rt == 0);
  if ((int32_t)cpu.gpr[operands->rs] <= 0) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgtzl) {
  if ((int32_t)cpu.gpr[operands->rs] > 0) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bltzl) {
  if ((int32_t)cpu.gpr[operands->rs] < 0) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgezl) {
  if ((int32_t)cpu.gpr[operands->rs] >= 0) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgezall) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[operands->rs] >= 0) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bltzall) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[operands->rs] < 0) {
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    prepare_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

//////////////////////////////////////////////////////////////
//                      unlikely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beq) {
  if (cpu.gpr[operands->rs] == cpu.gpr[operands->rt])
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bne) {
  if (cpu.gpr[operands->rs] != cpu.gpr[operands->rt])
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(blez) {
  InstAssert(operands->rt == 0);
  if ((int32_t)cpu.gpr[operands->rs] <= 0)
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgtz) {
  if ((int32_t)cpu.gpr[operands->rs] > 0)
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bltz) {
  if ((int32_t)cpu.gpr[operands->rs] < 0)
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgez) {
  if ((int32_t)cpu.gpr[operands->rs] >= 0)
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bgezal) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[operands->rs] >= 0)
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(bltzal) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[operands->rs] < 0)
    cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  prepare_delayslot();
}

make_exec_handler(jal) {
  cpu.gpr[31] = cpu.pc + 8;
  cpu.br_target =
      (cpu.pc & 0xf0000000) | (operands->addr << 2);
#if CONFIG_FUNCTION_TRACE_LOG
  frames_enqueue_call(cpu.pc, cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(jalr) {
  InstAssert(operands->rt == 0 && operands->shamt == 0);
  cpu.gpr[operands->rd] = cpu.pc + 8;
  cpu.br_target = cpu.gpr[operands->rs];
#if CONFIG_FUNCTION_TRACE_LOG
  frames_enqueue_call(cpu.pc, cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(j) {
  cpu.br_target =
      (cpu.pc & 0xf0000000) | (operands->addr << 2);
  prepare_delayslot();
}

make_exec_handler(jr) {
  InstAssert(operands->rt == 0 && operands->rd == 0);
  cpu.br_target = cpu.gpr[operands->rs];
#if CONFIG_FUNCTION_TRACE_LOG
  if (operands->rs == R_ra)
    frames_enqueue_ret(cpu.pc, cpu.br_target);
#endif
  prepare_delayslot();
}

make_exec_handler(seb) {
  cpu.gpr[operands->rd] =
      (int32_t)(int8_t)cpu.gpr[operands->rt];
}

make_exec_handler(seh) {
  cpu.gpr[operands->rd] =
      (int32_t)(int16_t)cpu.gpr[operands->rt];
}

make_exec_handler(wsbh) {
  uint32_t rt_val = cpu.gpr[operands->rt];
  cpu.gpr[operands->rd] = ((rt_val & 0x00FF0000) << 8) |
                          ((rt_val & 0xFF000000) >> 8) |
                          ((rt_val & 0x000000FF) << 8) |
                          ((rt_val & 0x0000FF00) >> 8);
}

make_exec_handler(ins) {
  uint32_t lsb = operands->shamt;
  uint32_t msb = operands->rd;
  assert(lsb <= msb);
  uint32_t rs_val = cpu.gpr[operands->rs];
  uint32_t rt_val = cpu.gpr[operands->rt];
  uint32_t mask = (1ull << (msb - lsb + 1)) - 1;

  cpu.gpr[operands->rt] =
      (rt_val & ~(mask << lsb)) | ((rs_val & mask) << lsb);
}

make_exec_handler(ext) {
  uint32_t lsb = operands->shamt;
  uint32_t msbd = operands->rd;
  uint32_t size = msbd + 1;
  uint32_t rs_val = cpu.gpr[operands->rs];
  uint32_t mask = (1ull << size) - 1;

  cpu.gpr[operands->rt] = ((rs_val & (mask << lsb)) >> lsb);
}

make_exec_handler(mfc1) {
  cpu.gpr[operands->rt] = cpu.fpr32i[operands->fs];
}
make_exec_handler(cfc1) {
  uint32_t fs = operands->fs;
  fcsr_t *rt = (fcsr_t *)&cpu.gpr[operands->rt];
  rt->val = 0;
  if (fs == 0) {
    InstAssert(0);
  } else if (fs == 25) {
    rt->fcc1_7 = cpu.fcsr.fcc1_7;
    rt->fcc0 = cpu.fcsr.fcc0;
  } else if (fs == 26) {
    rt->flags = cpu.fcsr.flags;
    rt->causes = cpu.fcsr.causes;
  } else if (fs == 28) {
    rt->RM = cpu.fcsr.RM;
    rt->flags |= cpu.fcsr.fcc0;
    rt->enables = cpu.fcsr.enables;
  } else if (fs == 31) {
    *rt = cpu.fcsr;
  }
}
make_exec_handler(mfhc1) {
  cpu.gpr[operands->rt] = cpu.fpr32i[operands->fs | 1];
}
make_exec_handler(mtc1) {
  cpu.fpr32i[operands->fs] = cpu.gpr[operands->rt];
}
make_exec_handler(ctc1) {
  /* copy a word to fpu control register */
  uint32_t fs = operands->fs;
  uint32_t rt_val = cpu.gpr[operands->rt];
  fcsr_t *rt = (fcsr_t *)&rt_val;
  if (fs == 25) {
    cpu.fcsr.fcc0 = rt_val & 0x1;
    cpu.fcsr.fcc1_7 = (rt_val >> 1) & 0x7F;
  } else if (fs == 26) {
    cpu.fcsr.flags = rt->flags;
    cpu.fcsr.causes = rt->causes;
  } else if (fs == 28) {
    cpu.fcsr.RM = rt->RM;
    cpu.fcsr.enables = rt->enables;
    cpu.fcsr.fs = (rt_val & 0x4) >> 2;
  } else if (fs == 31) {
    cpu.fcsr.val = rt_val;
  }
}
make_exec_handler(mthc1) {
  cpu.fpr32i[operands->fs | 1] = cpu.gpr[operands->rt];
}
make_exec_handler(bc1) {
  InstAssert(operands->nd == 0);
  if (operands->tf == 0) {
    /* bc1f */
    if (getFPCondCode(operands->cc2) == 0)
      cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    else
      cpu.br_target = cpu.pc + 8;
  } else if (operands->tf == 1) {
    /* bc1t */
    if (getFPCondCode(operands->cc2) == 1)
      cpu.br_target = cpu.pc + (operands->simm << 2) + 4;
    else
      cpu.br_target = cpu.pc + 8;
  }

  prepare_delayslot();
}

make_exec_handler(add_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_add(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(add_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_add(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}
make_exec_handler(sub_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_sub(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(sub_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_sub(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}
make_exec_handler(mul_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_mul(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(mul_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_mul(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}
make_exec_handler(div_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(
      float32_div(make_float32(cpu.fpr32i[operands->fs]),
          make_float32(cpu.fpr32i[operands->ft]), &status));
}
make_exec_handler(div_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_div(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      make_float64(cpu.fpr64i[operands->ft >> 1]),
      &status));
}

make_exec_handler(sqrt_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(float32_sqrt(
      make_float32(cpu.fpr32i[operands->fs]), &status));
}
make_exec_handler(sqrt_d) {
  float_status status = {};
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_sqrt(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      &status));
}

make_exec_handler(abs_s) {
  cpu.fpr32i[operands->fd] = float32_val(
      float32_abs(make_float32(cpu.fpr32i[operands->fs])));
}
make_exec_handler(abs_d) {
  cpu.fpr64i[operands->fd >> 1] = float64_val(float64_abs(
      make_float64(cpu.fpr64i[operands->fs >> 1])));
}

make_exec_handler(mov_s) {
  cpu.fpr32i[operands->fd] = cpu.fpr32i[operands->fs];
}
make_exec_handler(mov_d) {
  cpu.fpr64i[operands->fd >> 1] =
      cpu.fpr64i[operands->fs >> 1];
}

make_exec_handler(neg_s) {
  cpu.fpr32i[operands->fd] =
      cpu.fpr32i[operands->fs] ^ (1u << 31);
}
make_exec_handler(neg_d) {
  cpu.fpr64i[operands->fd >> 1] =
      cpu.fpr64i[operands->fs >> 1] ^ (1ull << 63);
}

make_exec_handler(trunc_w_s) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(float32_to_int32(
      make_float32(cpu.fpr32i[operands->fs]), &status));
}
make_exec_handler(trunc_w_d) {
  float_status status = {};
  cpu.fpr32i[operands->fd] = float32_val(float64_to_int32(
      make_float64(cpu.fpr64i[operands->fs >> 1]),
      &status));
}

make_exec_handler(movci) {
  if (operands->tf == 0) {
    /* movf */
    if (getFPCondCode(operands->cc2) == 0)
      cpu.gpr[operands->rd] = cpu.gpr[operands->rs];
  } else if (operands->tf == 1) {
    /* movt */
    if (getFPCondCode(operands->cc2) == 1)
      cpu.gpr[operands->rd] = cpu.gpr[operands->rs];
  }
}

make_exec_handler(movcf_s) {
  if (getFPCondCode(operands->cc2) == 0) {
    cpu.fpr32i[operands->fd] = cpu.fpr32i[operands->fs];
  }
}
make_exec_handler(movcf_d) {
  if (getFPCondCode(operands->cc2) == 0) {
    cpu.fpr64i[operands->fd64] = cpu.fpr32i[operands->fs64];
  }
}

make_exec_handler(movz_s) {
  if (cpu.gpr[operands->rt] == 0) {
    cpu.fpr32i[operands->fd] = cpu.fpr32i[operands->fs];
  }
}

make_exec_handler(movz_d) {
  if (cpu.gpr[operands->rt] == 0) {
    cpu.fpr64i[operands->fd64] = cpu.fpr64i[operands->fs64];
  }
}
make_exec_handler(movn_s) {
  if (cpu.gpr[operands->rt] != 0) {
    *(float *)&cpu.fpr32i[operands->fd] =
        *(float *)&cpu.fpr32i[operands->fs];
  }
}
make_exec_handler(movn_d) {
  if (cpu.gpr[operands->rt] != 0) {
    *(double *)&cpu.fpr32i[operands->fd & ~1] =
        *(double *)&cpu.fpr32i[operands->fs & ~1];
  }
}
make_exec_handler(cvt_d_s) {
  *(double *)&cpu.fpr32i[operands->fd & ~1] =
      *(float *)&cpu.fpr32i[operands->fs];
}
make_exec_handler(cvt_w_s) {
  cpu.fpr32i[operands->fd] =
      (int32_t) * (float *)&cpu.fpr32i[operands->fs];
}
make_exec_handler(cvt_s_d) {
  *(float *)&cpu.fpr32i[operands->fd] =
      *(double *)&cpu.fpr32i[operands->fs & ~1];
}
make_exec_handler(cvt_w_d) {
  cpu.fpr32i[operands->fd] =
      (int32_t) * (double *)&cpu.fpr32i[operands->fs & ~1];
}
make_exec_handler(cvt_s_w) {
  *(float *)&cpu.fpr32i[operands->fd] =
      (int32_t)cpu.fpr32i[operands->fs];
}
make_exec_handler(cvt_d_w) {
  *(double *)&cpu.fpr32i[operands->fd & ~1] =
      (int32_t)cpu.fpr32i[operands->fs];
}
make_exec_handler(c_un_s) {
  setFPCondCode(operands->cc1, 0);
}
make_exec_handler(c_un_d) {
  setFPCondCode(operands->cc1, 0);
}
make_exec_handler(c_eq_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] == cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_eq_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] ==
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ueq_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] == cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ueq_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] ==
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_olt_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] < cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_olt_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ult_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] < cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ult_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ole_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] <= cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ole_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <=
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_ule_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] <= cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_ule_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <=
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_lt_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] < cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_lt_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <
                         cpu.fpr64f[operands->ft64]);
}
make_exec_handler(c_le_s) {
  setFPCondCode(operands->cc1,
      cpu.fpr32f[operands->fs] <= cpu.fpr32f[operands->ft]);
}
make_exec_handler(c_le_d) {
  setFPCondCode(
      operands->cc1, cpu.fpr64f[operands->fs64] <=
                         cpu.fpr64f[operands->ft64]);
}

make_exec_handler(lwc1) {
  CHECK_ALIGNED_ADDR_AdEL(
      4, cpu.gpr[operands->rs] + operands->simm);
  uint32_t rdata =
      vaddr_read(cpu.gpr[operands->rs] + operands->simm, 4);
  if (!cpu.has_exception) {
    cpu.fpr32i[operands->rt] = rdata;
  }
}

make_exec_handler(swc1) {
  uint32_t waddr = cpu.gpr[operands->rs] + operands->simm;
  vaddr_write(waddr, 4, cpu.fpr32i[operands->ft]);
}

make_exec_handler(ldc1) {
  uint32_t raddr = cpu.gpr[operands->rs] + operands->simm;
  CHECK_ALIGNED_ADDR_AdEL(8, raddr);

  uint32_t rdata_l = vaddr_read(raddr, 4);
  uint32_t rdata_h = vaddr_read(raddr + 4, 4);
  if (!cpu.has_exception) {
    cpu.fpr32i[operands->rt & ~1] = rdata_l;
    cpu.fpr32i[operands->rt | 1] = rdata_h;
  }
}

make_exec_handler(sdc1) {
  uint32_t waddr = cpu.gpr[operands->rs] + operands->simm;
  CHECK_ALIGNED_ADDR_AdES(8, waddr);
  vaddr_write(waddr, 4, cpu.fpr32i[operands->ft & ~1]);
  vaddr_write(waddr + 4, 4, cpu.fpr32i[operands->ft | 1]);
}

#if CONFIG_DELAYSLOT
make_label(inst_end) {
  if (cpu.is_delayslot) {
    cpu.pc = cpu.br_target;
    cpu.is_delayslot = false;
  } else {
    cpu.pc += 4;
  }
  /* fall through */
}
#endif

make_exit() {
#if 0
  if (cpu.gpr[0] != 0)
    eprintf("%08x: set zero to %08x\n", get_current_pc(), cpu.gpr[0]);
#endif
}
