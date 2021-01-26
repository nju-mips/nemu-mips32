#if 1
make_exec_handler(exec_special) {
  panic("should not reach here !");
  // goto *special_table[ops->func];
}

make_exec_handler(exec_special2) {
  panic("should not reach here !");
  // goto *special2_table[ops->func];
}

make_exec_handler(exec_special3) {
  panic("should not reach here !");
  // goto *special3_table[ops->func];
}

make_exec_handler(exec_bshfl) {
  panic("should not reach here !");
  // goto *bshfl_table[I_SA];
}

make_exec_handler(exec_regimm) {
  panic("should not reach here !");
  // goto *regimm_table[GR_T];
}

make_exec_handler(exec_cop0) {
  panic("should not reach here !");
  /*
  if (GR_S & 0x10)
    goto *cop0_table_func[ops->func];
  else
    goto *cop0_table_rs[GR_S];
  */
}

make_exec_handler(exec_cop1) {
  panic("should not reach here !");
  /*
  if (GR_S == FPU_FMT_S)
    goto *cop1_table_rs_S[ops->func];
  else if (GR_S == FPU_FMT_D)
    goto *cop1_table_rs_D[ops->func];
  else if (GR_S == FPU_FMT_W)
    goto *cop1_table_rs_W[ops->func];
  else
    goto *cop1_table_rs[GR_S];
  */
}
#endif

make_exec_handler(inv) {
// the pc corresponding to this inst
// pc has been updated by instr_fetch
#if CONFIG_EXCEPTION
  raise_exception(EXC_RI);
#else
  uint32_t instr = vaddr_read(local_cpu.pc, 4);
  uint8_t *p = (uint8_t *)&instr;
  printf(
      "invalid opcode(pc = 0x%08x): %02x %02x %02x %02x "
      "...\n",
      local_cpu.pc, p[0], p[1], p[2], p[3]);
  nemu_state = NEMU_END;
#endif
}
