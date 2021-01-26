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

