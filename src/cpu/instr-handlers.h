#define make_label(l) \
  l:
#define make_entry()
#define make_exit() make_label(exit)

#define make_exec_handler(name) \
  goto inst_end;                \
  make_label(name)

#define exec_delayslot()   \
  cpu.is_delayslot = true; \
  cpu.pc += 4;             \
  goto exit;

#ifdef ENABLE_EXCEPTION
#  define InstAssert(cond)         \
    do {                           \
      if (!(cond)) {               \
        cpu.cp0.badvaddr = cpu.pc; \
        signal_exception(EXC_RI);  \
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
} L64_t;

/* clang-format off */
/* R-type */
static const void *special_table[64] = {
    /* 0x00 */ &&sll, &&inv, &&srl, &&sra,
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
    /* 0x00 */ &&inv, &&inv, &&mul, &&inv,
    /* 0x04 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x08 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x0c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x10 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x14 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x20 */ &&clz, &&inv, &&inv, &&inv,
    /* 0x24 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x28 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x2c */ &&inv, &&inv, &&inv, &&inv,
    /* 0x30 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
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
    /* 0x1c */ &&inv, &&inv, &&inv, &&inv,
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

    /* 0x20 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x24 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x28 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x2c */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x30 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x34 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x38 */ &&inv,  &&inv,  &&inv,   &&inv,
    /* 0x3c */ &&inv,  &&inv,  &&inv,   &&inv,
};

/* I-type */
static const void *opcode_table[64] = {
    /* 0x00 */ &&exec_special, &&exec_regimm, &&j, &&jal,
    /* 0x04 */ &&beq, &&bne, &&blez, &&bgtz,
    /* 0x08 */ &&addi, &&addiu, &&slti, &&sltiu,
    /* 0x0c */ &&andi, &&ori, &&xori, &&lui,
    /* 0x10 */ &&exec_cop0, &&inv, &&inv, &&inv,
    /* 0x14 */ &&beql, &&bnel, &&blezl, &&bgtzl,
    /* 0x18 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x1c */ &&exec_special2, &&inv, &&inv, &&inv,
    /* 0x20 */ &&lb, &&lh, &&lwl, &&lw,
    /* 0x24 */ &&lbu, &&lhu, &&lwr, &&inv,
    /* 0x28 */ &&sb, &&sh, &&swl, &&sw,
    /* 0x2c */ &&inv, &&inv, &&swr, &&cache,
    /* 0x30 */ &&ll, &&inv, &&inv, &&pref,
    /* 0x34 */ &&inv, &&inv, &&inv, &&inv,
    /* 0x38 */ &&sc, &&inv, &&inv, &&inv,
    /* 0x3c */ &&inv, &&inv, &&inv, &&inv,
};

/* clang-format on */

make_entry() {
#ifdef ENABLE_DECODE_CACHE_PERF
  decode_cache_hit += !!decode->handler;
  decode_cache_miss += !decode->handler;
#endif

  if (decode->handler) {
#ifdef ENABLE_INSTR_LOG
    instr_enqueue_instr(decode->inst.val);
#endif

#ifdef ENABLE_DECODE_CACHE_CHECK
    assert (decode->inst.val == dbg_vaddr_read(cpu.pc, 4));
#endif
    goto *(decode->handler);
  }

  Inst inst = {.val = load_mem(cpu.pc, 4)};
#if defined(ENABLE_INSTR_LOG) || defined(ENABLE_DECODE_CACHE_CHECK)
  decode->inst.val = load_mem(cpu.pc, 4);
#endif
#ifdef ENABLE_INSTR_LOG
  instr_enqueue_instr(decode->inst.val);
#endif

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
  } while (0);

Handler:
  goto *(decode->handler);
}

#if 1
/* make_entry() { goto *opcode_table[inst.op]; } */

make_exec_handler(exec_special) { goto *special_table[decode->func]; }

make_exec_handler(exec_special2) { goto *special2_table[decode->func]; }

make_exec_handler(exec_regimm) { goto *regimm_table[decode->rt]; }

make_exec_handler(exec_cop0) {
  if (decode->rs & 0x10)
    goto *cop0_table_func[decode->func];
  else
    goto *cop0_table_rs[decode->rs];
}
#endif

make_exec_handler(inv) {
// the pc corresponding to this inst
// pc has been updated by instr_fetch
#ifdef ENABLE_EXCEPTION
  signal_exception(EXC_RI);
#else
  uint32_t instr = load_mem(cpu.pc, 4);
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
  CPUAssert(i < NR_TLB_ENTRY, "invalid tlb index %d (%d)\n", i, NR_TLB_ENTRY);
  tlb_write(i);
  clear_mmu_cache();
  clear_decode_cache();
}

make_exec_handler(tlbwr) {
  uint32_t i = rand() % NR_TLB_ENTRY;
  cpu.cp0.random = i;
  tlb_write(i);
  clear_mmu_cache();
  clear_decode_cache();
}

/* temporary strategy: store timer registers in C0 */
make_exec_handler(syscall) { signal_exception(EXC_SYSCALL); }

make_exec_handler(breakpoint) {
  if (work_mode == MODE_GDB) {
    nemu_state = NEMU_STOP;
  } else {
    signal_exception(EXC_BP);
  }
}

make_exec_handler(eret) {
  cpu.has_exception = true;

  cpu.br_target = cpu.cp0.epc;
  cpu.cp0.cause.BD = 0;
  cpu.cp0.status.EXL = 0;

#ifdef ENABLE_CAE_CHECK
  check_usual_registers();
#endif

#ifdef ENABLE_SEGMENT
  cpu.base = cpu.cp0.reserved[CP0_RESERVED_BASE];
#endif
}

#define CPRS(reg, sel) (((reg) << 3) | (sel))

make_exec_handler(mfc0) {
#ifdef ENABLE_INTR
  cpu.gpr[decode->rt] = cpu.cp0.cpr[decode->rd][decode->sel];
#else
  /* only for microbench */
  if (decode->rd == CP0_COUNT) {
    L64_t us;
    us.val = get_current_time() * 50; // for 50 MHZ
    if (decode->sel == 0) {
      cpu.gpr[decode->rt] = us.lo;
    } else if (decode->sel == 1) {
      cpu.gpr[decode->rt] = us.hi;
    } else {
      assert(0);
    }
  } else {
    cpu.gpr[decode->rt] = cpu.cp0.cpr[decode->rd][decode->sel];
  }
#endif
}

make_exec_handler(mtc0) {
  switch (CPRS(decode->rd, decode->sel)) {
  case CPRS(CP0_EBASE, CP0_EBASE_SEL):
    cpu.cp0.cpr[decode->rd][decode->sel] = cpu.gpr[decode->rt];
    break;
  case CPRS(CP0_BADVADDR, 0): break;
  case CPRS(CP0_STATUS, 0): {
    cp0_status_t *newVal = (void *)&(cpu.gpr[decode->rt]);
    cpu.cp0.status.CU = newVal->CU;
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
    cpu.cp0.compare = cpu.gpr[decode->rt];
    cpu.cp0.cause.IP &= ~(CAUSE_IP_TIMER);
    break;
  case CPRS(CP0_CAUSE, 0): {
    uint32_t sw_ip_mask = 3;
    cp0_cause_t *newVal = (void *)&(cpu.gpr[decode->rt]);
    cpu.cp0.cause.IV = newVal->IV;
    cpu.cp0.cause.WP = newVal->WP;
    cpu.cp0.cause.IP =
        (newVal->IP & sw_ip_mask) | (cpu.cp0.cause.IP & ~sw_ip_mask);
  } break;
  case CPRS(CP0_PAGEMASK, 0): {
    cp0_pagemask_t *newVal = (void *)&(cpu.gpr[decode->rt]);
    cpu.cp0.pagemask.mask = newVal->mask;
    break;
  }
  case CPRS(CP0_ENTRY_LO0, 0): {
    cp0_entry_lo_t *newVal = (void *)&(cpu.gpr[decode->rt]);
    cpu.cp0.entry_lo0.g = newVal->g;
    cpu.cp0.entry_lo0.v = newVal->v;
    cpu.cp0.entry_lo0.d = newVal->d;
    cpu.cp0.entry_lo0.c = newVal->c;
    cpu.cp0.entry_lo0.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_LO1, 0): {
    cp0_entry_lo_t *newVal = (void *)&(cpu.gpr[decode->rt]);
    cpu.cp0.entry_lo1.g = newVal->g;
    cpu.cp0.entry_lo1.v = newVal->v;
    cpu.cp0.entry_lo1.d = newVal->d;
    cpu.cp0.entry_lo1.c = newVal->c;
    cpu.cp0.entry_lo1.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_HI, 0): {
    cp0_entry_hi_t *newVal = (void *)&(cpu.gpr[decode->rt]);
    cpu.cp0.entry_hi.asid = newVal->asid;
    cpu.cp0.entry_hi.vpn = newVal->vpn;
    clear_mmu_cache();
    clear_decode_cache();
  } break;
  case CPRS(CP0_INDEX, 0): {
    cpu.cp0.index.idx = cpu.gpr[decode->rt];
  } break;
#ifdef ENABLE_KERNEL_DEBUG
  // this serial is for debugging,
  // please don't use it in real codes
  case CPRS(CP0_RESERVED, CP0_RESERVED_SERIAL): {
    putchar(cpu.gpr[decode->rt]);
    break;
  }
  case CPRS(CP0_RESERVED, CP0_RESERVED_CHECK): {
    check_kernel_image(KERNEL_ELF_PATH);
    break;
  }
#endif
  default: cpu.cp0.cpr[decode->rd][decode->sel] = cpu.gpr[decode->rt]; break;
  }
}

make_exec_handler(teq) {
  if ((int32_t)cpu.gpr[decode->rs] == (int32_t)cpu.gpr[decode->rt]) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(teqi) {
  if ((int32_t)cpu.gpr[decode->rs] == decode->simm) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tge) {
  if ((int32_t)cpu.gpr[decode->rs] >= (int32_t)cpu.gpr[decode->rt]) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tgei) {
  if ((int32_t)cpu.gpr[decode->rs] >= decode->simm) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tgeiu) {
  if (cpu.gpr[decode->rs] >= decode->simm) { signal_exception(EXC_TRAP); }
}

make_exec_handler(tgeu) {
  if (cpu.gpr[decode->rs] >= cpu.gpr[decode->rt]) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tlt) {
  if ((int32_t)cpu.gpr[decode->rs] < (int32_t)cpu.gpr[decode->rt]) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tlti) {
  if ((int32_t)cpu.gpr[decode->rs] < decode->simm) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tltiu) {
  if (cpu.gpr[decode->rs] < decode->simm) { signal_exception(EXC_TRAP); }
}

make_exec_handler(tltu) {
  if (cpu.gpr[decode->rs] < cpu.gpr[decode->rt]) { signal_exception(EXC_TRAP); }
}

make_exec_handler(tne) {
  if ((int32_t)cpu.gpr[decode->rs] != (int32_t)cpu.gpr[decode->rt]) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(tnei) {
  if ((int32_t)cpu.gpr[decode->rs] != decode->simm) {
    signal_exception(EXC_TRAP);
  }
}

make_exec_handler(jr) {
  InstAssert(decode->rt == 0 && decode->rd == 0);
  cpu.br_target = cpu.gpr[decode->rs];
  exec_delayslot();
}

#define R_SIMPLE(name, op, t)                                               \
  make_exec_handler(name) {                                                 \
    InstAssert(decode->shamt == 0);                                         \
    cpu.gpr[decode->rd] = (t)cpu.gpr[decode->rs] op(t) cpu.gpr[decode->rt]; \
  }

R_SIMPLE(or, |, uint32_t)
R_SIMPLE (xor, ^, uint32_t)
R_SIMPLE(and, &, uint32_t)
R_SIMPLE(addu, +, uint32_t)
R_SIMPLE(subu, -, uint32_t)
R_SIMPLE(mul, *, uint32_t)
R_SIMPLE(slt, <, int32_t)
R_SIMPLE(sltu, <, uint32_t)

make_exec_handler(add) {
  InstAssert(decode->shamt == 0);
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[decode->rs] +
            (int64_t)(int32_t)cpu.gpr[decode->rt];
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#ifdef ENABLE_EXCEPTION
    signal_exception(EXC_OV);
#else
    CPUAssert(0, "add overflow, %08x + %08x\n", cpu.gpr[decode->rs],
        cpu.gpr[decode->rt]);
#endif
  } else {
    cpu.gpr[decode->rd] = ret.lo;
  }
}

make_exec_handler(sub) {
  InstAssert(decode->shamt == 0);
  L64_t ret;
  ret.val = (int64_t)(int32_t)cpu.gpr[decode->rs] -
            (int64_t)(int32_t)cpu.gpr[decode->rt];
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#ifdef ENABLE_EXCEPTION
    signal_exception(EXC_OV);
#else
    CPUAssert(0, "sub overflow, %08x - %08x\n", cpu.gpr[decode->rs],
        cpu.gpr[decode->rt]);
#endif
  } else {
    cpu.gpr[decode->rd] = ret.lo;
  }
}

make_exec_handler(nor) {
  InstAssert(decode->shamt == 0);
  cpu.gpr[decode->rd] = ~(cpu.gpr[decode->rs] | cpu.gpr[decode->rt]);
}

#undef R_SIMPLE

make_exec_handler(clz) {
  if (cpu.gpr[decode->rs] == 0) {
    cpu.gpr[decode->rd] = 32;
  } else {
    cpu.gpr[decode->rd] = __builtin_clz(cpu.gpr[decode->rs]);
  }
}

make_exec_handler(mult) {
  InstAssert(decode->rd == 0 && decode->shamt == 0);
  int64_t prod = (int64_t)(int32_t)cpu.gpr[decode->rs] *
                 (int64_t)(int32_t)cpu.gpr[decode->rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
}

make_exec_handler(multu) {
  InstAssert(decode->rd == 0 && decode->shamt == 0);
  uint64_t prod = (uint64_t)cpu.gpr[decode->rs] * (uint64_t)cpu.gpr[decode->rt];
  cpu.lo = (uint32_t)prod;
  cpu.hi = (uint32_t)(prod >> 32);
}

make_exec_handler(divide) {
  InstAssert(decode->rd == 0 && decode->shamt == 0);
  cpu.lo = (int32_t)cpu.gpr[decode->rs] / (int32_t)cpu.gpr[decode->rt];
  cpu.hi = (int32_t)cpu.gpr[decode->rs] % (int32_t)cpu.gpr[decode->rt];
}

make_exec_handler(divu) {
  InstAssert(decode->rd == 0 && decode->shamt == 0);
  cpu.lo = cpu.gpr[decode->rs] / cpu.gpr[decode->rt];
  cpu.hi = cpu.gpr[decode->rs] % cpu.gpr[decode->rt];
}

make_exec_handler(sll) {
  InstAssert(decode->rs == 0);
  cpu.gpr[decode->rd] = cpu.gpr[decode->rt] << decode->shamt;
}

make_exec_handler(sllv) {
  InstAssert(decode->shamt == 0);
  cpu.gpr[decode->rd] = cpu.gpr[decode->rt] << (cpu.gpr[decode->rs] & 0x1f);
}

make_exec_handler(sra) {
  InstAssert(decode->rs == 0);
  cpu.gpr[decode->rd] = (int32_t)cpu.gpr[decode->rt] >> decode->shamt;
}

make_exec_handler(srav) {
  InstAssert(decode->shamt == 0);
  cpu.gpr[decode->rd] =
      (int32_t)cpu.gpr[decode->rt] >> (cpu.gpr[decode->rs] & 0x1f);
}

make_exec_handler(srl) {
  InstAssert(decode->rs == 0);
  cpu.gpr[decode->rd] = cpu.gpr[decode->rt] >> decode->shamt;
}

make_exec_handler(srlv) {
  InstAssert(decode->shamt == 0);
  cpu.gpr[decode->rd] = cpu.gpr[decode->rt] >> (cpu.gpr[decode->rs] & 0x1f);
}

make_exec_handler(movn) {
  InstAssert(decode->shamt == 0);
  if (cpu.gpr[decode->rt] != 0) cpu.gpr[decode->rd] = cpu.gpr[decode->rs];
}

make_exec_handler(movz) {
  InstAssert(decode->shamt == 0);
  if (cpu.gpr[decode->rt] == 0) cpu.gpr[decode->rd] = cpu.gpr[decode->rs];
}

make_exec_handler(mfhi) {
  InstAssert(decode->rs == 0 && decode->rt == 0 && decode->shamt == 0);
  cpu.gpr[decode->rd] = cpu.hi;
}

make_exec_handler(mthi) {
  InstAssert(decode->rt == 0 && decode->rd == 0 && decode->shamt == 0);
  cpu.hi = cpu.gpr[decode->rs];
}

make_exec_handler(mflo) {
  InstAssert(decode->rs == 0 && decode->rt == 0 && decode->shamt == 0);
  cpu.gpr[decode->rd] = cpu.lo;
}

make_exec_handler(mtlo) {
  InstAssert(decode->rt == 0 && decode->rd == 0 && decode->shamt == 0);
  cpu.lo = cpu.gpr[decode->rs];
}

make_exec_handler(jalr) {
  InstAssert(decode->rt == 0 && decode->shamt == 0);
  cpu.gpr[decode->rd] = cpu.pc + 8;
  cpu.br_target = cpu.gpr[decode->rs];
  exec_delayslot();
}

make_exec_handler(lui) {
  InstAssert(decode->rs == 0);
  cpu.gpr[decode->rt] = decode->uimm << 16;
}

make_exec_handler(addi) {
  // should throw exception
  L64_t ret;
  ret.val =
      (int64_t)(int32_t)cpu.gpr[decode->rs] + (int64_t)(int32_t)decode->simm;
  if ((ret.hi & 0x1) != ((ret.lo >> 31) & 1)) {
#ifdef ENABLE_EXCEPTION
    signal_exception(EXC_OV);
#else
    CPUAssert(
        0, "addi overflow, %08x + %08x\n", cpu.gpr[decode->rs], decode->simm);
#endif
  } else {
    cpu.gpr[decode->rt] = ret.lo;
  }
}

make_exec_handler(addiu) {
  cpu.gpr[decode->rt] = cpu.gpr[decode->rs] + decode->simm;
}

make_exec_handler(andi) {
  cpu.gpr[decode->rt] = cpu.gpr[decode->rs] & decode->uimm;
}

make_exec_handler(ori) {
  cpu.gpr[decode->rt] = cpu.gpr[decode->rs] | decode->uimm;
}

make_exec_handler(xori) {
  cpu.gpr[decode->rt] = cpu.gpr[decode->rs] ^ decode->uimm;
}

make_exec_handler(sltiu) {
  cpu.gpr[decode->rt] = cpu.gpr[decode->rs] < decode->simm;
}

make_exec_handler(slti) {
  cpu.gpr[decode->rt] = (int32_t)cpu.gpr[decode->rs] < decode->simm;
}

#ifdef ENABLE_EXCEPTION

#  define CHECK_ALIGNED_ADDR_AdEL(align, addr) \
    if (((addr) & (align - 1)) != 0) {         \
      cpu.cp0.badvaddr = addr;                 \
      signal_exception(EXC_AdEL);              \
      goto exit;                               \
    }

#  define CHECK_ALIGNED_ADDR_AdES(align, addr) \
    if (((addr) & (align - 1)) != 0) {         \
      cpu.cp0.badvaddr = addr;                 \
      signal_exception(EXC_AdES);              \
      goto exit;                               \
    }

#else

#  define CHECK_ALIGNED_ADDR(align, addr)  \
    CPUAssert(((addr) & (align - 1)) == 0, \
        "address(0x%08x) is unaligned, pc=%08x\n", (addr), cpu.pc)

#  define CHECK_ALIGNED_ADDR_AdEL CHECK_ALIGNED_ADDR
#  define CHECK_ALIGNED_ADDR_AdES CHECK_ALIGNED_ADDR

#endif

make_exec_handler(swl) {
  uint32_t waddr = cpu.gpr[decode->rs] + decode->simm;
  int idx = waddr & 0x3;
  int len = idx + 1;
  uint32_t wdata = cpu.gpr[decode->rt] >> ((3 - idx) * 8);

  store_mem((waddr >> 2) << 2, len, wdata);
}

make_exec_handler(swr) {
  uint32_t waddr = cpu.gpr[decode->rs] + decode->simm;
  int len = 4 - (waddr & 0x3);
  uint32_t wdata = cpu.gpr[decode->rt];

  store_mem(waddr, len, wdata);
}

make_exec_handler(sw) {
  CHECK_ALIGNED_ADDR_AdES(4, cpu.gpr[decode->rs] + decode->simm);
  store_mem(cpu.gpr[decode->rs] + decode->simm, 4, cpu.gpr[decode->rt]);
}

make_exec_handler(sh) {
  CHECK_ALIGNED_ADDR_AdES(2, cpu.gpr[decode->rs] + decode->simm);
  store_mem(cpu.gpr[decode->rs] + decode->simm, 2, cpu.gpr[decode->rt]);
}

make_exec_handler(sb) {
  CHECK_ALIGNED_ADDR_AdES(1, cpu.gpr[decode->rs] + decode->simm);
  store_mem(cpu.gpr[decode->rs] + decode->simm, 1, cpu.gpr[decode->rt]);
}

make_exec_handler(lwl) {
  uint32_t raddr = cpu.gpr[decode->rs] + decode->simm;
  int len = (raddr & 0x3) + 1;
  uint32_t rdata = load_mem((raddr >> 2) << 2, len);

  if (!cpu.has_exception) {
    if (len < 4)
      cpu.gpr[decode->rt] =
          rdata << ((4 - len) * 8) |
          ((uint32_t)cpu.gpr[decode->rt] << (len * 8)) >> (len * 8);
    else
      cpu.gpr[decode->rt] = rdata;
  }
}

make_exec_handler(lwr) {
  uint32_t raddr = cpu.gpr[decode->rs] + decode->simm;
  int idx = raddr & 0x3;
  int len = 4 - idx;
  uint32_t rdata = load_mem(raddr, len);
  if (!cpu.has_exception) {
    if (len < 4)
      cpu.gpr[decode->rt] = (rdata << idx * 8) >> (idx * 8) |
                            ((uint32_t)cpu.gpr[decode->rt] >> (len * 8))
                                << (len * 8);
    else
      cpu.gpr[decode->rt] = (rdata << idx * 8) >> (idx * 8);
  }
}

make_exec_handler(lw) {
  CHECK_ALIGNED_ADDR_AdEL(4, cpu.gpr[decode->rs] + decode->simm);
  uint32_t rdata = load_mem(cpu.gpr[decode->rs] + decode->simm, 4);
  if (!cpu.has_exception) { cpu.gpr[decode->rt] = rdata; }
}

make_exec_handler(lb) {
  CHECK_ALIGNED_ADDR_AdEL(1, cpu.gpr[decode->rs] + decode->simm);
  uint32_t rdata =
      (int32_t)(int8_t)load_mem(cpu.gpr[decode->rs] + decode->simm, 1);
  if (!cpu.has_exception) { cpu.gpr[decode->rt] = rdata; }
}

make_exec_handler(lbu) {
  CHECK_ALIGNED_ADDR_AdEL(1, cpu.gpr[decode->rs] + decode->simm);
  uint32_t rdata = load_mem(cpu.gpr[decode->rs] + decode->simm, 1);
  if (!cpu.has_exception) { cpu.gpr[decode->rt] = rdata; }
}

make_exec_handler(lh) {
  CHECK_ALIGNED_ADDR_AdEL(2, cpu.gpr[decode->rs] + decode->simm);
  uint32_t rdata =
      (int32_t)(int16_t)load_mem(cpu.gpr[decode->rs] + decode->simm, 2);
  if (!cpu.has_exception) { cpu.gpr[decode->rt] = rdata; }
}

make_exec_handler(lhu) {
  CHECK_ALIGNED_ADDR_AdEL(2, cpu.gpr[decode->rs] + decode->simm);
  uint32_t rdata = load_mem(cpu.gpr[decode->rs] + decode->simm, 2);
  if (!cpu.has_exception) { cpu.gpr[decode->rt] = rdata; }
}

make_exec_handler(pref) {}

make_exec_handler(ll) {
  CHECK_ALIGNED_ADDR_AdEL(4, cpu.gpr[decode->rs] + decode->simm);
  cpu.gpr[decode->rt] = vaddr_read(cpu.gpr[decode->rs] + decode->simm, 4);
}

make_exec_handler(sc) {
  CHECK_ALIGNED_ADDR_AdES(4, cpu.gpr[decode->rs] + decode->simm);
  vaddr_write(cpu.gpr[decode->rs] + decode->simm, 4, cpu.gpr[decode->rt]);
  if (!cpu.has_exception) cpu.gpr[decode->rt] = 1;
}

make_exec_handler(cache) { clear_decode_cache(); }

make_exec_handler(sync) {}

//////////////////////////////////////////////////////////////
//                      likely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beql) {
  if (cpu.gpr[decode->rs] == cpu.gpr[decode->rt]) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bnel) {
  if (cpu.gpr[decode->rs] != cpu.gpr[decode->rt]) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(blezl) {
  InstAssert(decode->rt == 0);
  if ((int32_t)cpu.gpr[decode->rs] <= 0) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgtzl) {
  if ((int32_t)cpu.gpr[decode->rs] > 0) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bltzl) {
  if ((int32_t)cpu.gpr[decode->rs] < 0) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgezl) {
  if ((int32_t)cpu.gpr[decode->rs] >= 0) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bgezall) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[decode->rs] >= 0) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

make_exec_handler(bltzall) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[decode->rs] < 0) {
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
    exec_delayslot();
  } else {
    cpu.br_target = cpu.pc + 8;
    cpu.pc += 4;
  }
}

//////////////////////////////////////////////////////////////
//                      unlikely branch //
//////////////////////////////////////////////////////////////
make_exec_handler(beq) {
  if (cpu.gpr[decode->rs] == cpu.gpr[decode->rt])
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(bne) {
  if (cpu.gpr[decode->rs] != cpu.gpr[decode->rt])
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(blez) {
  InstAssert(decode->rt == 0);
  if ((int32_t)cpu.gpr[decode->rs] <= 0)
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(bgtz) {
  if ((int32_t)cpu.gpr[decode->rs] > 0)
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(bltz) {
  if ((int32_t)cpu.gpr[decode->rs] < 0)
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(bgez) {
  if ((int32_t)cpu.gpr[decode->rs] >= 0)
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(bgezal) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[decode->rs] >= 0)
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(bltzal) {
  cpu.gpr[31] = cpu.pc + 8;
  if ((int32_t)cpu.gpr[decode->rs] < 0)
    cpu.br_target = cpu.pc + (decode->simm << 2) + 4;
  else
    cpu.br_target = cpu.pc + 8;
  exec_delayslot();
}

make_exec_handler(jal) {
  cpu.gpr[31] = cpu.pc + 8;
  cpu.br_target = (cpu.pc & 0xf0000000) | (decode->addr << 2);
  exec_delayslot();
}

make_exec_handler(j) {
  cpu.br_target = (cpu.pc & 0xf0000000) | (decode->addr << 2);
  exec_delayslot();
}

make_label(inst_end) {
  if (cpu.is_delayslot) {
    cpu.pc = cpu.br_target;
    cpu.is_delayslot = false;
  } else {
    cpu.pc += 4;
  }
  /* fall through */
}

make_exit();
