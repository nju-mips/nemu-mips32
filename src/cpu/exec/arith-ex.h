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

