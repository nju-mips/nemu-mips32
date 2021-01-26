make_exec_handler(teq) {
  if ((int32_t)cpu.gpr[operands->rs] ==
      (int32_t)cpu.gpr[operands->rt]) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(teqi) {
  if ((int32_t)cpu.gpr[operands->rs] == operands->simm) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tge) {
  if ((int32_t)cpu.gpr[operands->rs] >=
      (int32_t)cpu.gpr[operands->rt]) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tgei) {
  if ((int32_t)cpu.gpr[operands->rs] >= operands->simm) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tgeiu) {
  if (cpu.gpr[operands->rs] >= operands->simm) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tgeu) {
  if (cpu.gpr[operands->rs] >= cpu.gpr[operands->rt]) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tlt) {
  if ((int32_t)cpu.gpr[operands->rs] <
      (int32_t)cpu.gpr[operands->rt]) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tlti) {
  if ((int32_t)cpu.gpr[operands->rs] < operands->simm) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tltiu) {
  if (cpu.gpr[operands->rs] < operands->simm) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tltu) {
  if (cpu.gpr[operands->rs] < cpu.gpr[operands->rt]) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tne) {
  if ((int32_t)cpu.gpr[operands->rs] !=
      (int32_t)cpu.gpr[operands->rt]) {
    raise_exception(EXC_TRAP);
  }
}

make_exec_handler(tnei) {
  if ((int32_t)cpu.gpr[operands->rs] != operands->simm) {
    raise_exception(EXC_TRAP);
  }
}

