make_exec_handler(sltiu) {
  cpu.gpr[operands->rt] =
      cpu.gpr[operands->rs] < operands->simm;
}

make_exec_handler(slti) {
  cpu.gpr[operands->rt] =
      (int32_t)cpu.gpr[operands->rs] < operands->simm;
}

R_SIMPLE(slt, <, int32_t)
R_SIMPLE(sltu, <, uint32_t)

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
