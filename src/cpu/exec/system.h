make_exec_handler(cache) { clear_decode_cache(); }
make_exec_handler(sync) {}
make_exec_handler(synci) {}

/* tlb strategy */
make_exec_handler(tlbp) { tlb_present(); }

make_exec_handler(tlbr) {
  uint32_t i = local_cpu.cp0.index.idx;
  CPUAssert(i < NR_TLB_ENTRY, "invalid tlb index\n");
  tlb_read(i);
}

make_exec_handler(tlbwi) {
  uint32_t i = local_cpu.cp0.index.idx;
  CPUAssert(i < NR_TLB_ENTRY, "invalid tlb index %d (%d)\n",
      i, NR_TLB_ENTRY);
  tlb_write(i);
  clear_mmu_cache();
  clear_decode_cache();
}

make_exec_handler(tlbwr) {
#if CONFIG_MARCH_NOOP
  tlb_write(local_cpu.cp0.random);
  local_cpu.cp0.random = (local_cpu.cp0.random + 1) % NR_TLB_ENTRY;
#else
  local_cpu.cp0.random = rand() % NR_TLB_ENTRY;
  tlb_write(local_cpu.cp0.random);
#endif
  clear_mmu_cache();
  clear_decode_cache();
}

/* temporary strategy: store timer registers in C0 */
make_exec_handler(syscall) {
  raise_exception(EXC_SYSCALL);
#if CONFIG_DUMP_SYSCALL
  local_cpu.is_syscall = true;
  void dump_syscall(
      uint32_t v0, uint32_t a0, uint32_t a1, uint32_t a2);
  dump_syscall(local_cpu.gpr[R_v0], local_cpu.gpr[R_a0], local_cpu.gpr[R_a1],
      local_cpu.gpr[R_a2]);
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
  local_cpu.has_exception = true;

  if (local_cpu.cp0.status.ERL == 1) {
    local_cpu.br_target = local_cpu.cp0.cpr[CP0_ErrorEPC][0];
    local_cpu.cp0.status.ERL = 0;
  } else {
    local_cpu.br_target = local_cpu.cp0.epc;
    local_cpu.cp0.status.EXL = 0;
  }

#if 0
  eprintf("%08x: ERET to %08x: ERL %d, EXL %d\n", local_cpu.pc, local_cpu.br_target,
      local_cpu.cp0.status.ERL, local_cpu.cp0.status.EXL);
#endif

#if CONFIG_DUMP_SYSCALL
  if (local_cpu.is_syscall) {
    printf("==> v0: %08x & %d\n", local_cpu.gpr[R_v0],
        local_cpu.gpr[R_v0]);
    local_cpu.is_syscall = false;
  }
#endif

  clear_mmu_cache();
  clear_decode_cache();
}

#define CPRS(reg, sel) (((reg) << 3) | (sel))

make_exec_handler(mfc0) {
  /* used for nanos: pal and litenes */
  if (GR_D == CP0_COUNT) {
    GR_TV = local_cpu.cp0.count[0];
  } else {
    GR_TV = local_cpu.cp0.cpr[GR_D][ops->sel];
  }
}

make_exec_handler(mtc0) {
  switch (CPRS(GR_D, ops->sel)) {
  case CPRS(CP0_EBASE, CP0_EBASE_SEL):
  case CPRS(CP0_COUNT, 0):
  case CPRS(CP0_EPC, 0):
    local_cpu.cp0.cpr[GR_D][ops->sel] = GR_TV;
    break;
  case CPRS(CP0_BADVADDR, 0): break;
  case CPRS(CP0_CONTEXT, 0): {
    cp0_context_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.context.PTEBase = newVal->PTEBase;
  } break;
  case CPRS(CP0_CONFIG, 0): {
    cp0_config_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.config.K0 = newVal->K0;
  } break;
  case CPRS(CP0_STATUS, 0): {
    cp0_status_t *newVal = (void *)&(GR_TV);
    if (local_cpu.cp0.status.ERL != newVal->ERL) {
      clear_decode_cache();
      clear_mmu_cache();
    }
    local_cpu.cp0.status.CU = newVal->CU & 0x3;
    local_cpu.cp0.status.RP = newVal->RP;
    local_cpu.cp0.status.RE = newVal->RE;
    local_cpu.cp0.status.BEV = newVal->BEV;
    local_cpu.cp0.status.TS = newVal->TS;
    local_cpu.cp0.status.SR = newVal->SR;
    local_cpu.cp0.status.NMI = newVal->NMI;
    local_cpu.cp0.status.IM = newVal->IM;
    local_cpu.cp0.status.UM = newVal->UM;
    local_cpu.cp0.status.ERL = newVal->ERL;
    local_cpu.cp0.status.EXL = newVal->EXL;
    local_cpu.cp0.status.IE = newVal->IE;
  } break;
  case CPRS(CP0_COMPARE, 0):
    local_cpu.cp0.compare = GR_TV;
    nemu_set_irq(7, 0);
    break;
  case CPRS(CP0_CAUSE, 0): {
    cp0_cause_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.cause.IV = newVal->IV;
    local_cpu.cp0.cause.WP = newVal->WP;

    nemu_set_irq(0, newVal->IP & (1 << 0));
    nemu_set_irq(1, newVal->IP & (1 << 1));
  } break;
  case CPRS(CP0_PAGEMASK, 0): {
    cp0_pagemask_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.pagemask.mask = newVal->mask;
    break;
  }
  case CPRS(CP0_ENTRY_LO0, 0): {
    cp0_entry_lo_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.entry_lo0.g = newVal->g;
    local_cpu.cp0.entry_lo0.v = newVal->v;
    local_cpu.cp0.entry_lo0.d = newVal->d;
    local_cpu.cp0.entry_lo0.c = newVal->c;
    local_cpu.cp0.entry_lo0.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_LO1, 0): {
    cp0_entry_lo_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.entry_lo1.g = newVal->g;
    local_cpu.cp0.entry_lo1.v = newVal->v;
    local_cpu.cp0.entry_lo1.d = newVal->d;
    local_cpu.cp0.entry_lo1.c = newVal->c;
    local_cpu.cp0.entry_lo1.pfn = newVal->pfn;
  } break;
  case CPRS(CP0_ENTRY_HI, 0): {
    cp0_entry_hi_t *newVal = (void *)&(GR_TV);
    local_cpu.cp0.entry_hi.asid = newVal->asid;
    local_cpu.cp0.entry_hi.vpn = newVal->vpn;
    clear_mmu_cache();
    clear_decode_cache();
  } break;
  case CPRS(CP0_INDEX, 0): {
    local_cpu.cp0.index.idx = GR_TV;
  } break;
  // this serial is for debugging,
  // please don't use it in real codes
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
    printf("%08x: mtc0 $%s, $%d, %d\n", local_cpu.pc, regs[GR_T],
        GR_D, ops->sel);
    break;
  }
}
