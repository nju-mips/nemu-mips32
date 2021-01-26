make_exec_handler(cache) { clear_decode_cache(); }
make_exec_handler(sync) {}
make_exec_handler(synci) {}

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
  raise_exception(EXC_SYSCALL);
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
    raise_exception(EXC_BP);
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

#if CONFIG_SEGMENT
  cpu.base = cpu.cp0.reserved[CP0_RESERVED_BASE];
#endif

  clear_mmu_cache();
  clear_decode_cache();
}

#define CPRS(reg, sel) (((reg) << 3) | (sel))

make_exec_handler(mfc0) {
  /* used for nanos: pal and litenes */
  if (GR_D == CP0_COUNT) {
    GR_TV = cpu.cp0.count[0];
  } else {
    GR_TV = cpu.cp0.cpr[GR_D][operands->sel];
  }
}

make_exec_handler(mtc0) {
  switch (CPRS(GR_D, operands->sel)) {
  case CPRS(CP0_EBASE, CP0_EBASE_SEL):
  case CPRS(CP0_COUNT, 0):
  case CPRS(CP0_EPC, 0):
    cpu.cp0.cpr[GR_D][operands->sel] = GR_TV;
    break;
  case CPRS(CP0_BADVADDR, 0): break;
  case CPRS(CP0_CONTEXT, 0): {
    cp0_context_t *newVal = (void *)&(GR_TV);
    cpu.cp0.context.PTEBase = newVal->PTEBase;
  } break;
  case CPRS(CP0_CONFIG, 0): {
    cp0_config_t *newVal = (void *)&(GR_TV);
    cpu.cp0.config.K0 = newVal->K0;
  } break;
  case CPRS(CP0_STATUS, 0): {
    cp0_status_t *newVal = (void *)&(GR_TV);
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
    cpu.cp0.compare = GR_TV;
    nemu_set_irq(7, 0);
    break;
  case CPRS(CP0_CAUSE, 0): {
    cp0_cause_t *newVal = (void *)&(GR_TV);
    cpu.cp0.cause.IV = newVal->IV;
    cpu.cp0.cause.WP = newVal->WP;

    nemu_set_irq(0, newVal->IP & (1 << 0));
    nemu_set_irq(1, newVal->IP & (1 << 1));
  } break;
  case CPRS(CP0_PAGEMASK, 0): {
    cp0_pagemask_t *newVal = (void *)&(GR_TV);
    cpu.cp0.pagemask.mask = newVal->mask;
    break;
  }
  case CPRS(CP0_ENTRY_LO0, 0): {
    cp0_entry_lo_t *newVal = (void *)&(GR_TV);
    cpu.cp0.entry_lo0.g = newVal->g;
    cpu.cp0.entry_lo0.v = newVal->v;
    cpu.cp0.entry_lo0.d = newVal->d;
    cpu.cp0.entry_lo0.c = newVal->c;
    cpu.cp0.entry_lo0.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_LO1, 0): {
    cp0_entry_lo_t *newVal = (void *)&(GR_TV);
    cpu.cp0.entry_lo1.g = newVal->g;
    cpu.cp0.entry_lo1.v = newVal->v;
    cpu.cp0.entry_lo1.d = newVal->d;
    cpu.cp0.entry_lo1.c = newVal->c;
    cpu.cp0.entry_lo1.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_HI, 0): {
    cp0_entry_hi_t *newVal = (void *)&(GR_TV);
    cpu.cp0.entry_hi.asid = newVal->asid;
    cpu.cp0.entry_hi.vpn = newVal->vpn;
    clear_mmu_cache();
    clear_decode_cache();
  } break;
  case CPRS(CP0_INDEX, 0): {
    cpu.cp0.index.idx = GR_TV;
  } break;
  // this serial is for debugging,
  // please don't use it in real codes
  case CPRS(CP0_RESERVED, CP0_RESERVED_BASE):
#if CONFIG_SEGMENT
    cpu.cp0.cpr[GR_D][operands->sel] = GR_TV;
#endif
    break;
  case CPRS(CP0_RESERVED, CP0_RESERVED_SERIAL): {
#if CONFIG_KERNEL_DEBUG_SERIAL
    putchar(GR_TV);
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
    if (GR_TV == 0)
      printf("\e[1;32mHIT GOOD TRAP\e[0m\n");
    else
      printf("\e[1;31mHIT BAD TRAP %d\e[0m\n", GR_TV);
    nemu_exit(0);
  } break;
  default:
    printf("%08x: mtc0 $%s, $%d, %d\n", cpu.pc, regs[GR_T],
        GR_D, operands->sel);
    break;
  }
}
