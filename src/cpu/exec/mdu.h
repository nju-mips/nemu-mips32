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
